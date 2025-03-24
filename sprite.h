#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>

class Sprite {
public:
    Sprite(int screenW, int screenH);
    ~Sprite();

    void AddFrame(const std::wstring& path);
    void Update();
    void Move(int dx);
    void Draw(Gdiplus::Graphics& g);

    void SetPosition(int x, int y);
    void SetHeight(int h);

    int GetHeight() const { return height; }
    int GetScreenHeight() const { return screenHeight; }
    int GetScreenWidth() const { return screenWidth; }

private:
    std::vector<Gdiplus::Image*> frames;
    int currentFrame = 0;

    int x = 0, y = 0;
    int width = 100, height = 100;

    int screenWidth = 0;
    int screenHeight = 0;
};
