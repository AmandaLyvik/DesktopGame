#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

ULONG_PTR gdiplusToken; // this is like a "session ID" you pass to GdiplusStartup and GdiplusShutdown

// Declares the window procedure function that will handle messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // Init GDI+
    GdiplusStartupInput gdiPlusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiPlusStartupInput, nullptr);

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

    // MEMORY BITMAP FOR DRAWING
    // Make background transparent (using UpdateLayeredWindow)
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = screenWidth;
    bmi.bmiHeader.biHeight = -screenHeight; // top-down. aligns with normal screen coordinates (top left is (0,0))
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    SelectObject(hdcMem, hBitmap);

    // DRAW
    Graphics graphics(hdcMem);
    graphics.Clear(Color(0, 0, 0, 0)); // Fully transparent

    // get the sprite
    Image sprite(L"img/cat.png");
    int spriteWidth = sprite.GetWidth();
    int spriteHeight = sprite.GetHeight();
    
    // scale while maintianing aspect ratio
    int targetHeight = 150;
    float aspectRatio = static_cast<float>(spriteWidth) / spriteHeight;
    int targetWidth = static_cast<int>(targetHeight * aspectRatio);

    // position 
    int x = (screenWidth - targetWidth) / 2;
    int y = screenHeight - targetHeight - 50;

    graphics.DrawImage(&sprite, x, y, targetWidth, targetHeight);

    // Apply drawing
    POINT ptSrc = { 0, 0 };
    SIZE sizeWnd = { screenWidth, screenHeight };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    UpdateLayeredWindow(hwnd, hdcScreen, nullptr, &sizeWnd, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

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
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    return 0;
}

// Handles the WM_DESTROY message (sent when window closes), so app quits cleanly.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
