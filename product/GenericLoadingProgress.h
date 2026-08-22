// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <string>
#include <vector>

#include "InputDevice.h"
#include "RenderSystem.h"

class GenericLoadingProgress {
 public:
  GenericLoadingProgress(std::string BarName, std::string BarText,
                         std::vector<float> Position,
                         InputDevice* DeviceToAttachTo, int ThreadId);
  void ChangeVisibility(bool Visible);
  void MaintainScaling();

  // sets the filled fraction of the bar [0,1] - driven by the caller (e.g. an
  // asset load count), never by a press, so there is no callback to wire up
  void SetProgress(float Fraction);

 private:
  // track dimensions, matches GenericSlider's kSize
  static constexpr float kSize = 0.03f;

  std::string Name;
  std::string Text;

  std::vector<float> Pos;

  InputDevice* Device;

  int Id;

  // last set fraction along the track [0,1] - reapplied to the fill in
  // MaintainScaling so a resize doesn't reset the visible progress
  float Value = 0.f;
};
