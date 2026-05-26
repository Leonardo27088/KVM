#ifndef UNICODE
#define UNICODE
#endif

#include "window.h"

static const wchar_t windowClassName[] = L"myWindowClass";
HWND hwnd;

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