#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

using namespace Gdiplus;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    MessageBoxW(NULL, L"Hello WinAPI!", L"Overlay", MB_OK);
    return 0;
}
