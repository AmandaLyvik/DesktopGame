#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include "sprite.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

ULONG_PTR gdiplusToken; // this is like a "session ID" you pass to GdiplusStartup and GdiplusShutdown

// Declares the window procedure function that will handle messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


const int ANIMATION_TIMER_ID = 0;

const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
Sprite sprite(screenWidth, screenHeight);

void RedrawSprite(HWND hwnd) {
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = sprite.GetScreenWidth();
    bmi.bmiHeader.biHeight = -sprite.GetScreenHeight();
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    SelectObject(hdcMem, hBitmap);

    Graphics graphics(hdcMem);
    graphics.Clear(Color(0, 0, 0, 0)); // Transparent background

    try {
        sprite.Draw(graphics);
    } catch (...) {
        MessageBox(nullptr, L"Crash during Draw()!", L"Error", MB_OK);
    }
    
    // Apply to layered window
    SIZE wndSize = { screenWidth, screenHeight };
    POINT srcPt = { 0, 0 };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    UpdateLayeredWindow(hwnd, hdcScreen, nullptr, &wndSize, hdcMem, &srcPt, 0, &blend, ULW_ALPHA);

    // Cleanup
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // Init GDI+
    GdiplusStartupInput gdiPlusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiPlusStartupInput, nullptr);

    // Load animation frames
    sprite.LoadAnimations("animations");
    sprite.LoadStateMachine(L"stateMachine.json");

    // Set size
    sprite.SetHeight(150);

    // Starting position
    sprite.SetPosition(sprite.GetScreenWidth() - 3*sprite.GetWidth(), sprite.GetScreenHeight() - sprite.GetHeight() - 50);

    // THE WINDOW
    // Register window class
    WNDCLASSW wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"OverlayWindowClass";
    RegisterClassW(&wc);

    // Create layered, transparent, topmost window
    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST, //  per-pixel transparency | Click-through | stay above all other windows
        wc.lpszClassName,
        L"Overlay",
        WS_POPUP, // No border, no title bar
        0, 0, screenWidth, screenHeight,
        nullptr, nullptr, hInstance, nullptr
    );

    // Set timer
    SetTimer(hwnd, ANIMATION_TIMER_ID, 16, nullptr);  // ~60 FPS

    ShowWindow(hwnd, SW_SHOW);

    // Message loop
    // Keeps the window alive until closed. Even if you're not interacting with it, Windows needs this loop to process system events.
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    GdiplusShutdown(gdiplusToken);

    return 0;
}

// Handles the WM_DESTROY message (sent when window closes), so app quits cleanly.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_TIMER:
            if (wParam == ANIMATION_TIMER_ID) {
                sprite.Update(); // Handles animation + movement
                InvalidateRect(hwnd, nullptr, FALSE); // Redraw
            }
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);

            // Redraw overlay with new position + frame
            RedrawSprite(hwnd);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            KillTimer(hwnd, ANIMATION_TIMER_ID);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
