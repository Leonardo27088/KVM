#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    MSLLHOOKSTRUCT* lp = (MSLLHOOKSTRUCT*)lParam;

    switch(wParam) {
        case WM_LBUTTONDOWN:
            printf("Left Click\n");
            break;
        case WM_RBUTTONDOWN:
            printf("Right Click\n");
            break;
        case WM_MBUTTONDOWN:
            printf("Middle Click\n");
            break;
    }

    return CallNextHookEx(0, nCode, wParam, lParam);
}

int main() {
    HHOOK mouseHandle = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

    if (mouseHandle == NULL) {
        printf("Error");
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(mouseHandle);

    return 0;
}