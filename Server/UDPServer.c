#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "protocol.h"
#include "window.h"
#include "hooks.h"

#pragma comment (lib, "Ws2_32.lib");

#define DEFAULT_BUFLEN 512

SOCKET RecvSocket;
struct sockaddr_in RecvAddr;

struct sockaddr_in SenderAddr;
int SenderAddrSize = sizeof (SenderAddr);

MSG msg;

bool isRemote = false;
float virtualX = 1600.0f;
float virtualY = 900.0f;

int main() {
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

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
    RecvAddr.sin_port = htons(PORT);
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

    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (!CreateInvisibleWindow(hInstance)) {
        printf("Failed to initialize window\n");
        closesocket(RecvSocket);
        WSACleanup();
        return 1;
    }

    HHOOK mouseHandle = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

    if (mouseHandle == NULL) {
        printf("Error mouse hook\n");
        closesocket(RecvSocket);
        WSACleanup();
        return 1;
    }

    HHOOK keyboardHandle = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

    if (keyboardHandle == NULL) {
        printf("Error keyboard hook\n");
        closesocket(RecvSocket);
        WSACleanup();
        return 1;
    }

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    

    printf("Finished recieving. Closing socket.\n");
    iResult = closesocket(RecvSocket);
    if (iResult == SOCKET_ERROR) {
        printf("close socket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    UnhookWindowsHookEx(mouseHandle);
    UnhookWindowsHookEx(keyboardHandle);

    WSACleanup();
    return msg.wParam;
}