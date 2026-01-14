#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdio>
#include <thread>
#include <string>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#define NOE "10.4.113.85"
#define ME "192.168.1.14"

inline void receiveMessages(SOCKET connectSocket, uint32_t userId)
{
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    bool isChatting = true;

    while (isChatting)
    {
        iResult = recv(connectSocket, recvbuf, DEFAULT_BUFLEN, 0);
        if (iResult > 0)
        {
            recvbuf[iResult] = '\0';
            printf("%s", recvbuf);
            //std::println("{} {}", userId, recvbuf);
        }
        else if (iResult == 0)
        {
            printf("\nConnection closed\n");
            break;
        }
        else
        {
            printf("\nReceive error: %d\n", WSAGetLastError());
            break;
        }
    }
}

inline int startClient()
{
    WSADATA wsaData;
    int iResult;
    bool isChatting = true;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    struct addrinfo* result = NULL, * ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(ME, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    SOCKET connectSocket = INVALID_SOCKET;
    ptr = result;
    connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = connect(connectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
    if (iResult == SOCKET_ERROR)
    {
        closesocket(connectSocket);
        connectSocket = INVALID_SOCKET;
    }
    freeaddrinfo(result);

    if (connectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    printf("Connected! Type your message:\n");

    std::jthread recvThread(receiveMessages, connectSocket, 1);

    std::string message;
    while (isChatting)
    {
        std::getline(std::cin, message);

        if (message == "/quit")
        {
            printf("YOu left the chat");
            break;
        }

        message += " ";
        iResult = send(connectSocket, message.c_str(), static_cast<int>(message.length()), 0);
        if (iResult == SOCKET_ERROR)
        {
            printf("send failed: %d\n", WSAGetLastError());
            break;
        }
    }

    closesocket(connectSocket);
    WSACleanup();

    return 0;
}