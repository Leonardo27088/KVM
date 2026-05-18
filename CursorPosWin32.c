#include <windows.h>
#include <stdio.h>


// Variable global para el hook
HHOOK hMouseHook = NULL;

// Función de callback que procesa los eventos del ratón
LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        // Estructura que contiene la posición y datos del ratón
        MSLLHOOKSTRUCT *mouseStruct = (MSLLHOOKSTRUCT *)lParam;

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        // Filtramos para capturar solo el movimiento
        if (wParam == WM_MOUSEMOVE) {
            float normX = (float)mouseStruct->pt.x / screenWidth;
            float normY = (float)mouseStruct->pt.y / screenHeight;
        }
       if (wParam == WM_LBUTTONDOWN) {
        printf("Click");
        printf("Width: %d - Height: %d", screenWidth, screenHeight);
       }
       if (wParam == WM_RBUTTONDOWN) {
        while(ShowCursor(FALSE) >= 0);
       }
    }
    // Es crucial llamar al siguiente hook en la cadena
    return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

int main() {
    // Instalamos el Low-Level Mouse Hook
    // WH_MOUSE_LL no requiere estar en una DLL externa
    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, GetModuleHandle(NULL), 0);

    if (hMouseHook == NULL) {
        fprintf(stderr, "Error: No se pudo instalar el hook (%lu)\n", GetLastError());
        return 1;
    }

    printf("Capturando movimiento del mouse... (Presiona Ctrl+C para salir)\n");

    // Ciclo de mensajes: Necesario para que el sistema notifique a nuestra función
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Liberamos el hook antes de cerrar
    UnhookWindowsHookEx(hMouseHook);

    return 0;
}