#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdio>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512
#pragma comment(lib, "ws2_32.lib")

inline int startServeur()
{
	WSADATA wsaData;
	int iResult, iSendResult;
	char recvbf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

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
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET listenSocket = INVALID_SOCKET;
	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);

	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("Listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	SOCKET clientSocket;
	clientSocket = INVALID_SOCKET;

	clientSocket = accept(listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	closesocket(listenSocket);

	do
	{
		iResult = recv(clientSocket, recvbf, recvbuflen, 0);
		if (iResult > 0)
		{
			printf("Bytes received: %d\n", iResult);

			iSendResult = send(clientSocket, recvbf, iResult, 0);
			if (iSendResult == SOCKET_ERROR)
			{
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(clientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else
		{
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	iResult = shutdown(clientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}