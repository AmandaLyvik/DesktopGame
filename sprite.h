#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>


class Sprite {
public:
    Sprite(int screenW, int screenH);
    ~Sprite();

    void LoadFromJson(const std::wstring& jsonPath);

    void Update();  // Called every tick (e.g. 16ms)
    void Move(int dx);
    void Draw(Gdiplus::Graphics& g);

    void SetPosition(int x, int y);
    void SetHeight(int h);

    int GetHeight() const { return height; }
    int GetWidth() const { return width; }
    int GetScreenHeight() const { return screenHeight; }
    int GetScreenWidth() const { return screenWidth; }

private:
    struct Frame {
        Gdiplus::Image* image;
        int durationMs;
    };
    std::vector<Frame> frames;
    int currentFrame = 0;

    int screenWidth;
    int screenHeight;
    int x = 0, y = 0;
    int width = 100, height = 100;
    int movementX = 0;
    int movementY = 0;

    DWORD lastUpdateTime = 0;
    int elapsedSinceLastFrame = 0;

    Gdiplus::Image* GetCurrentFrameImage() const;
};
