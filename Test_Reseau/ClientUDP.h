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

inline void stillAlive(SOCKET connectSocket)
{
    std::string pulseMessage = "p";
}

inline int startClientUDP()
{
    WSADATA wsaData;
    SOCKET sendSocket = INVALID_SOCKET;
    sockaddr_in RecvAddr;

    int iResult;
    bool isChatting = true;
    unsigned short Port = 27015;

    char sendBuf[DEFAULT_BUFLEN];
    int bufLen = DEFAULT_BUFLEN;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sendSocket == INVALID_SOCKET)
    {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    inet_pton(AF_INET, NOE, &RecvAddr.sin_addr);

    wprintf(L"Sending test datagram\n");
    iResult = sendto(sendSocket, sendBuf, bufLen, 0,
        (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));

    if (iResult == SOCKET_ERROR)
    {
        wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
        closesocket(sendSocket);
        WSACleanup();
        return 1;
    }

    wprintf(L"Finished sending. Closing socket.\n");
    iResult = closesocket(sendSocket);
    if (iResult == SOCKET_ERROR)
    {
        wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    wprintf(L"Exiting.\n");
    WSACleanup();

    return 0;
}