#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>

class Sprite
{
public:
    Sprite(int screenW, int screenH);
    ~Sprite();

    //void LoadFromJson(const std::wstring &jsonPath);
    void LoadStateMachine(const std::wstring &stateMachinePath);
    void LoadAnimations(const std::string& folder);

    void Update(); // Called every tick (e.g. 16ms)
    void Move(int dx, int dy);
    void Draw(Gdiplus::Graphics &g);

    void SetPosition(int x, int y);
    void SetHeight(int h);

    int GetHeight() const { return height; }
    int GetWidth() const { return width; }
    int GetScreenHeight() const { return screenHeight; }
    int GetScreenWidth() const { return screenWidth; }

private:
    struct Frame
    {
        Gdiplus::Image *image;
        int durationMs;
    };
    struct Transition {
        std::string to;
        std::string condition;
        float probability = 1.0f;
        int intervalMin = 0;  // Minimum wait time for "randomInterval"
        int intervalMax = 0;  // Maximum wait time for "randomInterval"
        int intervalSet = 0;  // Exact wait time for "setInterval"
    };
    struct State {
        std::string animation;
        std::vector<Transition> transitions;
    };

    std::map<std::string, std::vector<Frame>> loadedAnimations; // Store animations
    std::map<std::string, std::pair<int, int>> animationMovements; // dx, dy
    std::map<std::string, State> stateMachine;  // State machine with transitions
    std::unordered_map<std::string, DWORD> animationStartTimes;

    int currentFrame = 0;
    std::string currentAnimation;
    std::vector<Frame> currentFrames;

    int screenWidth;
    int screenHeight;
    int x = 0, y = 0;
    int width = 100, height = 100;
    int movementX = 0;
    int movementY = 0;

    DWORD lastUpdateTime = 0;
    int elapsedSinceLastFrame = 0;

    void LoadAnimation(const std::string& animationName, const std::wstring& animationPath);
    void ApplyAnimation(const std::string& animationName);
    void CheckTransition();
    void ApplyTransition(const std::string& targetAnimation);
    bool EvaluateCondition(const std::string& condition, const Transition& transition);
    Gdiplus::Image *GetCurrentFrameImage() const;
};
