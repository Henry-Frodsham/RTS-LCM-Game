// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <atomic>
#include <string>
#include <vector>

#include "InputDevice.h"
#include "RenderSystem.h"

class GenericLoadingProgress {
 public:
  GenericLoadingProgress(std::string BarName, std::vector<float> Position,
                         std::vector<float> Dimensions,
                         InputDevice* DeviceToAttachTo, int ThreadId);

  void ChangeVisibility(bool Visible);
  void MaintainScaling();

  void SetProgress(float Fraction);
  float GetProgress() const;

 private:
  void PushFill(float Fraction) const;

  std::string TrackName;
  std::string FillName;
  std::string OverlayName;

  std::vector<float> Pos{0.f, 0.f};
  std::vector<float> Size{0.f, 0.f};

  std::atomic<float> Progress{0.f};
};
