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
    uint16_t code;
    int32_t value;
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

bool isRemote = false;
float virtualX = 1600.0f;
float virtualY = 900.0f;

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

BOOL CreateInvisibleWindow(HINSTANCE hInstance) {
    WNDCLASSEX wcx = {0};

    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = WndProc;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.lpszClassName = windowClassName;

    if (!RegisterClassEx(&wcx)) {
        return FALSE;
    }

    hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        windowClassName,
        L"KVM",
        WS_POPUP, 0, 0,
        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return FALSE;
    }

    SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);

    ShowWindow(hwnd, SW_HIDE);
    return TRUE;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    MSLLHOOKSTRUCT* lp = (MSLLHOOKSTRUCT*)lParam;
    const char* status = NULL;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;

    if (!isRemote && wParam == WM_MOUSEMOVE && lp->pt.x <= 0) {
        isRemote = true;
        virtualX = 1600.0f;
        virtualY = 450.0f;

        ShowWindow(hwnd, SW_SHOW);
        SetCursorPos(centerX, centerY);
        return 1;
    }

    if (isRemote) {
        if (lp->pt.x == centerX && lp->pt.y == centerY) {
            return 1;
        }

        if (wParam == WM_MOUSEMOVE) {
            int deltaX = lp->pt.x - centerX;
            int deltaY = lp->pt.y - centerY;

            virtualX += deltaX;
            virtualY += deltaY;

            if (virtualX >= 1600.0f) {
                isRemote = false;
                ShowWindow(hwnd, SW_HIDE);
                SetCursorPos(1, centerY);
                return 1;
            }

            if (virtualX < 0) virtualX = 0;
            if (virtualY < 0) virtualY = 0;
            if (virtualY > 900) virtualY = 900;

            MousePacket packet;
            packet.type = 1;
            packet.normX = virtualX / 1600.0f;
            packet.normY = virtualY / 900.0f;
            packet.code = 0;
            packet.value = 0;

            sendto(RecvSocket, (const char *)&packet, sizeof(packet), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));

            SetCursorPos(centerX, centerY);
            return 1;
        } else {
            switch (wParam) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP: {
                    MousePacket packet;
                    
                    packet.type = 2;
                    packet.normX = 0;
                    packet.normY = 0;

                    if (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP) packet.code = 0x110;
                    if (wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP) packet.code = 0x111;

                    packet.value = (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) ? 1 : 0;

                    sendto(RecvSocket, (const char *)&packet, sizeof(packet), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));
                    break;
                }
                case WM_MOUSEHWHEEL: {
                    MousePacket packet;

                    packet.type = 3;
                    packet.normX = 0;
                    packet.normY = 0;
                    packet.code = 0x08;

                    short delta = (short)HIWORD(lp->mouseData);
                    packet.value = (delta > 0) ? 1 : -1;

                    sendto(RecvSocket, (const char *)&packet, sizeof(packet), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));
                    break;
                }
            }
        }

        SetCursorPos(centerX, centerY);
        return 1;
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
    if (!CreateInvisibleWindow(hInstance)) {
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