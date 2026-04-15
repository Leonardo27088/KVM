#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib");

#define DEFAULT_BUFLEN 512

SOCKET RecvSocket;
struct sockaddr_in RecvAddr;

struct sockaddr_in SenderAddr;
int SenderAddrSize = sizeof (SenderAddr);

MSG msg;

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    MSLLHOOKSTRUCT* lp = (MSLLHOOKSTRUCT*)lParam;
    const char* status = NULL;

    switch(wParam) {
        case WM_LBUTTONDOWN:
            status = "Left Click";
            break;
        case WM_RBUTTONDOWN:
            status = "Right Click";
            break;
        case WM_MBUTTONDOWN:
            status = "Middle Click";
            break;
    }

    if (status != NULL) {
        sendto(RecvSocket, status, (int)strlen(status), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));
    }

    return CallNextHookEx(0, nCode, wParam, lParam);
}

int main() {
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    unsigned short Port = 27015;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    char *message = "Hello Client";

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (RecvSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    iResult = bind(RecvSocket, (SOCKADDR *) & RecvAddr, sizeof(RecvAddr));
    if (iResult != 0) {
        printf("bind failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("Recieving datagrams...\n");

    iResult = recvfrom(RecvSocket, recvbuf, recvbuflen, 0, (SOCKADDR *) &SenderAddr, &SenderAddrSize);
    if (iResult == SOCKET_ERROR) {
        printf("recv failed with error %d\n", WSAGetLastError());
    }

    recvbuf[iResult] = '\0';
    puts(recvbuf);

    sendto(RecvSocket, message, strlen(message), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));
    printf("Hello client sent\n");

    HHOOK mouseHandle = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

    if (mouseHandle == NULL) {
        printf("Error");
        return 1;
    }

    GetMessage(&msg, NULL, 0, 0);

    printf("Finished recieving. Closing socket.\n");
    iResult = closesocket(RecvSocket);
    if (iResult == SOCKET_ERROR) {
        printf("close socket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    UnhookWindowsHookEx(mouseHandle);

    WSACleanup();
    return 0;
}