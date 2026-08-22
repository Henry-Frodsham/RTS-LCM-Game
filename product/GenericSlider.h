// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <string>
#include <vector>

#include "InputDevice.h"
#include "RenderSystem.h"

class GenericSlider {
 public:
  GenericSlider(std::string ButtonName, std::string ButtonText,
                std::vector<float> Position, std::vector<float> Size,
                float MaxValue, float StepPerPixel,
                InputDevice* DeviceToRespondTo,
                std::function<void(EventQueue&, float, float)> ChangeCallback,
                EventQueue* QueueForCallBack, int ThreadId);
  void ChangeVisibility(bool Visible);
  void MaintainScaling();

 private:
  // owns the visual response to a press - resizes the fill to match, then
  // forwards to the caller-supplied callback
  void HandlePress(EventQueue& Queue, float MouseX, float MouseY);

  // fraction of the track [0,1] the fill should be drawn at for the current
  // Value - separated out since both HandlePress and MaintainScaling need it
  float FillFraction() const;

  std::string Name;
  std::string Text;

  std::vector<float> Pos;

  // track width/height, fully caller-controlled rather than a fixed square
  std::vector<float> Size;

  // upper bound for Value, and how much Value increases per pixel the press
  // lands to the right of the track's left edge (pixels, not the normalised
  // 0-1 viewport space Pos/MouseX are given in, so HandlePress converts using
  // RenderSystem::GetRenderWindowDimensions())
  float MaxValue;
  float StepValue;

  InputDevice* Device;

  std::function<void(EventQueue&, float, float)> CallbackOnValueChange;

  EventQueue* CallBackQueue;
  int Id;

  // last computed value in [0, MaxValue] - reapplied to the fill in
  // MaintainScaling so a resize doesn't reset the visible slider position
  float Value = 0.f;
};
