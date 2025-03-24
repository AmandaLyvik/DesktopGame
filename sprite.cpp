#include "sprite.h"
#include <fstream>
#include <sstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;
using namespace Gdiplus;

Sprite::Sprite(int screenW, int screenH) : screenWidth(screenW), screenHeight(screenH) {}

Sprite::~Sprite() {
    for (auto& frame : frames) {
        delete frame.image;
    }
}

void Sprite::LoadFromJson(const std::wstring& jsonPath) {
    // Convert wstring to UTF-8 for reading the file
    std::ifstream file(std::string(jsonPath.begin(), jsonPath.end()));
    if (!file.is_open()) return;

    json j;
    file >> j;

    if (j.contains("movement")) {
        movementX = j["movement"].value("dx", 0);
        movementY = j["movement"].value("dy", 0);
    }

    for (auto& frame : j["frames"]) {
        std::string imgPath = frame["image"];
        int duration = frame["duration"];

        std::wstring wpath(imgPath.begin(), imgPath.end());
        Gdiplus::Image* img = new Gdiplus::Image(wpath.c_str());
        frames.push_back({ img, duration });
    }

    lastUpdateTime = GetTickCount(); // Start tracking animation time
}

void Sprite::Update() {
    if (frames.empty()) return;

    DWORD now = GetTickCount();
    int delta = now - lastUpdateTime;
    elapsedSinceLastFrame += delta;
    lastUpdateTime = now;

    if (elapsedSinceLastFrame >= frames[currentFrame].durationMs) {
        elapsedSinceLastFrame = 0;
        currentFrame = (currentFrame + 1) % frames.size();
    }

    x += movementX;
    y += movementY;
    if (x > screenWidth) x = -width;
}

Gdiplus::Image* Sprite::GetCurrentFrameImage() const {
    if (frames.empty()) return nullptr;
    return frames[currentFrame].image;
}

void Sprite::Move(int dx) {
    x += dx;
    if (x > screenWidth) x = -width;
}

void Sprite::SetPosition(int px, int py) {
    x = px;
    y = py;
}

void Sprite::SetHeight(int h) {
    height = h;
    if (!frames.empty()) {
        // Calculate aspect ratio from the first frame
        Gdiplus::Image* firstImage = frames[0].image;
        if (firstImage != nullptr) {
            // Aspect ratio = width / height of the first frame
            float aspectRatio = static_cast<float>(firstImage->GetWidth()) / firstImage->GetHeight();
            
            // Set width based on height and maintain aspect ratio
            width = static_cast<int>(height * aspectRatio);
        }
    }
}

void Sprite::Draw(Graphics& g) {
    Image* img = GetCurrentFrameImage();
    if (!img) return;
    g.DrawImage(img, x, y, width, height);
}
