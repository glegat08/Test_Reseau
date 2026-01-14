#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdio>
#include <thread>
#include <vector>
#include <mutex>
#include <string>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#pragma comment(lib, "ws2_32.lib")

inline std::vector<SOCKET> clients;
inline std::mutex clientsMutex;
inline bool isChatting = true;

inline void broadcastMessage(const char* message, int length, SOCKET senderSocket)
{
    std::scoped_lock lock(clientsMutex);
    for (SOCKET clientSocket : clients)
    {
        if (clientSocket != senderSocket)
        {
            send(clientSocket, message, length, 0);
        }
    }
}

inline void handleClient(SOCKET clientSocket, int clientId)
{
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;

    printf("Client %d connected!\n", clientId);

    std::string welcomeMsg = "Welcome to the chat! You are client " + std::to_string(clientId) + "\n";
    send(clientSocket, welcomeMsg.c_str(), static_cast<int>(welcomeMsg.length()), 0);

    std::string joinMsg = "Client " + std::to_string(clientId) + " joined the chat\n";
    broadcastMessage(joinMsg.c_str(), static_cast<int>(joinMsg.length()), clientSocket);

    while (isChatting)
    {
        iResult = recv(clientSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
        {
            recvbuf[iResult] = '\0';
            printf("Client %d: %s", clientId, recvbuf);

            std::string message = "Client " + std::to_string(clientId) + ": " + std::string(recvbuf);

            if (message.back() != '\n')
            {
                message += "\n";
            }

            {
                std::scoped_lock lock(clientsMutex);
                for (SOCKET client : clients)
                {
                    send(client, message.c_str(), static_cast<int>(message.length()), 0);
                }
            }
        }
        else if (iResult == 0)
        {
            printf("Client %d disconnecting...\n", clientId);
            break;
        }
        else
        {
            printf("recv failed for client %d: %d\n", clientId, WSAGetLastError());
            break;
        }
    }

    {
        std::scoped_lock lock(clientsMutex);
        for (auto it = clients.begin(); it != clients.end(); ++it)
        {
            if (*it == clientSocket)
            {
                clients.erase(it);
                break;
            }
        }
    }

    std::string leaveMsg = "Client " + std::to_string(clientId) + " left the chat\n";
    broadcastMessage(leaveMsg.c_str(), static_cast<int>(leaveMsg.length()), clientSocket);

    closesocket(clientSocket);
    printf("Client %d disconnected\n", clientId);
}

inline int startServeur()
{
    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    struct addrinfo* result = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = bind(listenSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %s...\n", DEFAULT_PORT);

    int clientId = 0;
    std::vector<std::jthread> clientThreads;

    while (isChatting)
    {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET)
        {
            printf("accept failed: %d\n", WSAGetLastError());
            continue;
        }

        {
            std::scoped_lock lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        clientId++;
        clientThreads.emplace_back(handleClient, clientSocket, clientId);

        printf("Active threads: %zu\n", clientThreads.size());
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}