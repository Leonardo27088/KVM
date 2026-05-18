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

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    float normX;
    float normY;
    uint8_t button;
} MousePacket;
#pragma pack(pop)

#pragma comment (lib, "Ws2_32.lib");

#define DEFAULT_BUFLEN 512

SOCKET RecvSocket;
struct sockaddr_in RecvAddr;

struct sockaddr_in SenderAddr;
int SenderAddrSize = sizeof (SenderAddr);

MSG msg;
HWND hwnd;

const wchar_t windowClassName[] = L"myWindowClass";

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

BOOL CreateInvisibleWindow(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEX wcx;

    wcx.cbSize = sizeof(wcx);
    wcx.style = 0;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcx.lpszMenuName = NULL;
    wcx.lpszClassName = windowClassName;
    wcx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wcx)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, windowClassName, L"KVM",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return TRUE;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    MSLLHOOKSTRUCT* lp = (MSLLHOOKSTRUCT*)lParam;
    const char* status = NULL;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

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
        case WM_MOUSEMOVE:
            MousePacket packet;
            packet.type = 1;
            packet.normX = (float)lp->pt.x / screenWidth;
            packet.normY = (float)lp->pt.y / screenHeight;
            packet.button = 0;

            sendto(RecvSocket, (const char *)&packet, sizeof(packet), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));
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

    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (!CreateInvisibleWindow(hInstance, SW_SHOW)) {
        printf("Failed to initialize window\n");
        closesocket(RecvSocket);
        WSACleanup();
        return 1;
    }

    HHOOK mouseHandle = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

    if (mouseHandle == NULL) {
        printf("Error");
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

    WSACleanup();
    return msg.wParam;
}