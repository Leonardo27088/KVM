#include "hooks.h"
#include "protocolServer.h"

extern SOCKET RecvSocket;
extern struct sockaddr_in SenderAddr;
extern HWND hwnd;
extern bool isRemote;
extern float virtualX;
extern float virtualY;

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    MSLLHOOKSTRUCT* lp = (MSLLHOOKSTRUCT*)lParam;

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
        if (wParam == WM_MOUSEMOVE && lp->pt.x == centerX && lp->pt.y == centerY) {
            return 1;
        }

        if (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP ||
            wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP ||
            wParam == WM_MBUTTONDOWN || wParam == WM_MBUTTONUP) {

            MousePacket packet = {0};

            packet.normX = 0;
            packet.normY = 0;

            packet.type = PKT_BTN;

            if (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONUP) packet.code = 0x110;
            if (wParam == WM_RBUTTONDOWN || wParam == WM_RBUTTONUP) packet.code = 0x111;
            if (wParam == WM_MBUTTONDOWN || wParam == WM_MBUTTONUP) packet.code = 0x112;

            packet.value = (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) ? 1 : 0;
            
            sendto(RecvSocket, (const char *)&packet, sizeof(packet), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));
        } else if (wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP) {
            MousePacket packet = {0};

            packet.normX = 0;
            packet.normY = 0;

            packet.type = PKT_SWIPE;
            packet.code = 0x14e;
            packet.value = (wParam == WM_XBUTTONDOWN) ? 1 : 0;

            sendto(RecvSocket, (const char *)&packet, sizeof(packet), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));
        } else if (wParam == WM_MOUSEWHEEL) {
            MousePacket packet = {0};

            packet.normX = 0;
            packet.normY = 0;
            packet.type = PKT_WHEEL;
            packet.code = 0x08;

            short delta = (short)HIWORD(lp->mouseData);
            packet.value = (delta > 0) ? -1 : 1;

            sendto(RecvSocket, (const char *)&packet, sizeof(packet), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));
        } else if (wParam == WM_MOUSEMOVE) {
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

            MousePacket packet = {0};

            packet.type = PKT_MOVE;
            packet.normX = virtualX / 1600.0f;
            packet.normY = virtualY / 900.0f;
            packet.code = 0;
            packet.value = 0;

            sendto(RecvSocket, (const char *)&packet, sizeof(packet), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));

            SetCursorPos(centerX, centerY);
            return 1;
        }

        SetCursorPos(centerX, centerY);
        return 1;
    }

    return CallNextHookEx(0, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (!isRemote) return CallNextHookEx(0, nCode, wParam, lParam);

    KBDLLHOOKSTRUCT* kp = (KBDLLHOOKSTRUCT*)lParam;

    KeyPacket packet = {0};
    packet.type = PKT_KEY;
    packet.code = kp->vkCode;

    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) packet.value = 1;
    else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) packet.value = 0;
    else return CallNextHookEx(0, nCode, wParam, lParam);

    sendto(RecvSocket, (const char *)&packet, sizeof(packet), 0, (SOCKADDR *) &SenderAddr, sizeof(SenderAddr));

    return 1;
}