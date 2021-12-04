#include "connection.h"

#include <windows.h>
#include <ws2tcpip.h>

#include <assert.h>
#include <stdio.h>

#define SHOULD_NOT_BE_HERE assert(0);

static WSADATA wsaData;

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int connection_init(struct connection_s* connection, char* ip, char* port)
{
    int iResult;
    struct addrinfo* result = NULL, * ptr = NULL, hints;

    connection->socket = INVALID_SOCKET;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo("192.168.0.250", port, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        connection->socket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (connection->socket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        iResult = connect(connection->socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connection->socket);
            connection->socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (connection->socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    return 0;
}

int connection_send(struct connection_s* connection, char* buffer, int length)
{
    int iResult;

    iResult = send(connection->socket, buffer, length, 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connection->socket);
        WSACleanup();
        return 1;
    }

    return 0;
}

int connection_receive(struct connection_s* connection, char* buffer, int length)
{
    int iResult;

    iResult = recv(connection->socket, buffer, length, 0);

    if (iResult < 0) {
        printf("recv failed with error: %d\n", WSAGetLastError());
        SHOULD_NOT_BE_HERE
        return -1;
    }

    return iResult;
}

int connection_close(struct connection_s* connection)
{
    int iResult;

    iResult = shutdown(connection->socket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(connection->socket);
        WSACleanup();
        return 1;
    }

    closesocket(connection->socket);
    WSACleanup();

    return 0;
}
