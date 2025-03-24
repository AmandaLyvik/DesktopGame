#include "sprite.h"
using namespace Gdiplus;

Sprite::Sprite(int screenW, int screenH) : screenWidth(screenW), screenHeight(screenH) {}

Sprite::~Sprite() {
    for (auto img : frames)
        delete img;
}

void Sprite::AddFrame(const std::wstring& path) {
    Image* img = new Image(path.c_str());
    frames.push_back(img);

    if (frames.size() == 1) {
        // Calculate width based on first frame
        float aspect = (float)img->GetWidth() / img->GetHeight();
        width = static_cast<int>(height * aspect);
    }
}

void Sprite::Update() {
    currentFrame = (currentFrame + 1) % frames.size();
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
        float aspect = (float)frames[0]->GetWidth() / frames[0]->GetHeight();
        width = static_cast<int>(height * aspect);
    }
}

void Sprite::Draw(Graphics& g) {
    if (!frames.empty()) {
        g.DrawImage(frames[currentFrame], x, y, width, height);
    }
}
