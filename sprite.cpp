#include "sprite.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;
#include "nlohmann/json.hpp"
using json = nlohmann::json;
using namespace Gdiplus;

Sprite::Sprite(int screenW, int screenH) : screenWidth(screenW), screenHeight(screenH) {}

Sprite::~Sprite()
{
  for (auto &frame : currentFrames)
  {
    delete frame.image;
  }
}

void Sprite::LoadStateMachine(const std::wstring& stateMachinePath) {
  std::ifstream file(stateMachinePath);
  if (!file.is_open()) {
      std::cerr << "Failed to open state machine file" << std::endl;
      return;
  }

  nlohmann::ordered_json j;
  file >> j;
  bool firstState = true;

  for (auto& state : j.items()) {
      const auto& stateName = state.key();
      const auto& stateData = state.value();

      State newState;
      newState.animation = stateData["animation"];
      
      // Load transitions
      for (const auto& transition : stateData["transitions"]) {
          Transition newTransition;
          newTransition.to = transition["to"];
          newTransition.condition = transition["condition"];
          if (transition.contains("probability")) {
            newTransition.probability = transition["probability"];
          }
          newState.transitions.push_back(newTransition);
      }

      stateMachine[stateName] = newState;

      // Apply the animation of the first state as default
      if (firstState) {
        ApplyAnimation(newState.animation);
        firstState = false;
      }
  }
}

void Sprite::LoadAnimations(const std::string& folder) {
  // Load all animation files in the animations folder
  for (const auto& entry : fs::directory_iterator(folder)) {
    if (entry.is_regular_file() && entry.path().extension() == L".json") {
        std::ifstream file(entry.path());
        if (!file.is_open()) {
            continue;  // skip if it can't open
        }

        nlohmann::json j;
        try {
            file >> j;
            std::string animationName = j["name"];  // read the "name" field
            LoadAnimation(animationName, entry.path());
        } catch (const std::exception& e) {
            std::cerr << "Error loading animation: " << e.what() << std::endl;
        }
    }
  }
}

void Sprite::LoadAnimation(const std::string& animationName, const std::wstring& animationPath) 
{
  if (loadedAnimations.find(animationName) != loadedAnimations.end()) {
    return; // If animation is already loaded, no need to load again
  }

  std::ifstream file(animationPath);
  if (!file.is_open()) {
    std::cerr << "Failed to open animation file" << std::endl;
    return;
  }
    
  nlohmann::json j;
  file >> j;

  std::vector<Frame> frames;
  for (const auto& frameData : j["frames"])
  {
    Frame frame;
    try {
      std::string imagePathStr = frameData["image"];
      std::wstring imagePath(imagePathStr.begin(), imagePathStr.end());

      frame.image = new Gdiplus::Image(imagePath.c_str());
    } catch (const std::exception& e) {
      std::cerr << "Failed to load image: " << e.what() << std::endl;
    }
    frame.durationMs = frameData["duration"];
    frames.push_back(frame);
  }

  // Save the loaded frames into the map
  loadedAnimations[animationName] = frames;

  int dx = j["movement"]["dx"];
  int dy = j["movement"]["dy"];
  animationMovements[animationName] = { dx, dy };
}

void Sprite::ApplyAnimation(const std::string& animationName) {
  if (loadedAnimations.find(animationName) == loadedAnimations.end()) return;

  currentAnimation = animationName;
  currentFrames = loadedAnimations[animationName];
  currentFrame = 0;
  lastUpdateTime = GetTickCount();
  elapsedSinceLastFrame = 0;

  auto movementIt = animationMovements.find(animationName);
  if (movementIt != animationMovements.end()) {
      movementX = movementIt->second.first;
      movementY = movementIt->second.second;
  }
}

void Sprite::CheckTransition() {
  const auto& currentState = stateMachine[currentAnimation];

  // Group transitions by condition
  std::unordered_map<std::string, std::vector<Transition>> conditionGroups;

  for (const auto& transition : currentState.transitions) {
      conditionGroups[transition.condition].push_back(transition);
  }

  // Evaluate each condition **only once**
  for (const auto& [condition, transitions] : conditionGroups) {
      if (EvaluateCondition(condition)) { 
          // Calculate total probability
          float totalWeight = 0.0f;
          for (const auto& t : transitions) {
              totalWeight += t.probability;
          }

          // Generate a random number in range [0, totalWeight]
          float r = static_cast<float>(rand()) / RAND_MAX * totalWeight;

          // Pick a transition based on probability
          float cumulative = 0.0f;
          for (const auto& t : transitions) {
              cumulative += t.probability;
              if (r <= cumulative) {
                  ApplyTransition(t.to);
                  return; // Only one transition should happen
              }
          }
      }
  }
}

bool Sprite::EvaluateCondition(const std::string& condition) {
  if (condition == "atEndOfScreen" && x + width >= screenWidth) return true;
  if (condition == "atStartOfScreen" && x <= 0) return true;
  // Add more conditions here if needed

  return false; // Default to false if the condition is unknown
}

void Sprite::ApplyTransition(const std::string& targetAnimation) {
  if (currentAnimation != targetAnimation &&
      loadedAnimations.find(targetAnimation) != loadedAnimations.end()) 
  {
    ApplyAnimation(targetAnimation);
  } 
}

void Sprite::Update()
{
  if (currentFrames.empty()) return;

  DWORD now = GetTickCount();
  int delta = now - lastUpdateTime;
  elapsedSinceLastFrame += delta;
  lastUpdateTime = now;

  if (elapsedSinceLastFrame >= currentFrames[currentFrame].durationMs)
  {
    elapsedSinceLastFrame = 0;
    currentFrame = (currentFrame + 1) % currentFrames.size();
  }

  Move(movementX, movementY);

  // Check for animation transitions
  CheckTransition();
}

Gdiplus::Image *Sprite::GetCurrentFrameImage() const
{
  if (currentFrames.empty() || currentFrame >= currentFrames.size()) return nullptr;
  return currentFrames[currentFrame].image;
}

void Sprite::Move(int dx, int dy)
{
  x += dx;
  y += dy;
  // if (x > screenWidth) x = -width;
  // Prevent the sprite from moving off the screen
  if (x < 0) x = 0;
  if (x + width > screenWidth) x = screenWidth - width;
}

void Sprite::SetPosition(int px, int py)
{
  x = px;
  y = py;
}

void Sprite::SetHeight(int h)
{
  height = h;
  if (!currentFrames.empty())
  {
    // Calculate aspect ratio from the first frame
    Gdiplus::Image *firstImage = currentFrames[0].image;
    if (firstImage != nullptr)
    {
      // Aspect ratio = width / height of the first frame
      float aspectRatio = static_cast<float>(firstImage->GetWidth()) / firstImage->GetHeight();

      // Set width based on height and maintain aspect ratio
      width = static_cast<int>(height * aspectRatio);
    }
  }
}

void Sprite::Draw(Graphics &g)
{
  Image *img = GetCurrentFrameImage();
  if (!img) return;
  g.DrawImage(img, x, y, width, height);
}
