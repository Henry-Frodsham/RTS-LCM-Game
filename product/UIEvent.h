#pragma once
#include <vector>

struct ToggleInteractionWheelEvent {
  std::vector<float> Position;
  ToggleInteractionWheelEvent(std::vector<float> Pos) : Position(Pos) {}
};
