#pragma once

#include <windows.h>

extern HWND hwnd;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CreateInvisibleWindow(HINSTANCE hInstance);
