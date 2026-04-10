// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <vector>

struct ToggleInteractionWheelEvent {
  std::vector<float> Position;
  explicit ToggleInteractionWheelEvent(std::vector<float> Pos)
      : Position(Pos) {}
};
