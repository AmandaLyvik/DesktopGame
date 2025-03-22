#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

ULONG_PTR gdiplusToken; // this is like a "session ID" you pass to GdiplusStartup and GdiplusShutdown

// Declares the window procedure function that will handle messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


const int ANIMATION_TIMER_ID = 1;
const int MOVEMENT_TIMER_ID = 2;

struct SpriteState {
    std::vector<Image*> frames;
    int currentFrame = 0;

    int x = 0;
    int y = 0;

    int width = 0;
    int height = 150; 

    int screenWidth = 0;
    int screenHeight = 0;

    std::wstring currentAnimation = L"walk";
};
SpriteState sprite;

void RedrawSprite(HWND hwnd) {
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = sprite.screenWidth;
    bmi.bmiHeader.biHeight = -sprite.screenHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pvBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
    SelectObject(hdcMem, hBitmap);

    Graphics graphics(hdcMem);
    graphics.Clear(Color(0, 0, 0, 0)); // Transparent background

    graphics.DrawImage(
        sprite.frames[sprite.currentFrame],
        sprite.x,
        sprite.y,
        sprite.width,
        sprite.height
    );
    
    // Apply to layered window
    SIZE wndSize = { sprite.screenWidth, sprite.screenHeight };
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
    sprite.frames.push_back(new Image(L"img/walk1.png"));
    sprite.frames.push_back(new Image(L"img/walk2.png"));

    // Set screen size
    sprite.screenWidth = GetSystemMetrics(SM_CXSCREEN);
    sprite.screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Calculate dimensions
    float aspect = (float)sprite.frames[0]->GetWidth() / sprite.frames[0]->GetHeight();
    sprite.width = static_cast<int>(sprite.height * aspect);

    // Starting position
    sprite.x = 0;
    sprite.y = sprite.screenHeight - sprite.height - 50;

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
        0, 0, sprite.screenWidth, sprite.screenHeight,
        nullptr, nullptr, hInstance, nullptr
    );

    // Set timers
    SetTimer(hwnd, ANIMATION_TIMER_ID, 150, nullptr);  // Switch frame every 150ms
    SetTimer(hwnd, MOVEMENT_TIMER_ID, 30, nullptr);    // Move every 30ms

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
                sprite.currentFrame = (sprite.currentFrame + 1) % sprite.frames.size();
            } else if (wParam == MOVEMENT_TIMER_ID) {
                sprite.x += 2; // Move sprite right
                if (sprite.x > sprite.screenWidth) sprite.x = -sprite.width; // Loop back
            }

            // Trigger redraw
            InvalidateRect(hwnd, nullptr, FALSE);
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
            KillTimer(hwnd, MOVEMENT_TIMER_ID);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
