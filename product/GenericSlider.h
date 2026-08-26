// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "InputDevice.h"
#include "RenderSystem.h"

class GenericSlider {
 public:
  GenericSlider(std::string SliderName, std::vector<float> Position,
                std::vector<float> Dimensions, float MaxSliderValue,
                InputDevice* DeviceToRespondTo,
                std::function<void(EventQueue&, float)> ChangeCallback,
                EventQueue* QueueForCallBack, int ThreadId);

  void ChangeVisibility(bool Visible);
  void MaintainScaling();

  float GetValue() const;
  void SetValue(float NewValue);

 private:
  struct SliderState {
    std::string TrackName;
    std::string FillName;
    std::string OverlayName;

    std::vector<float> Pos{0.f, 0.f};
    std::vector<float> Size{0.f, 0.f};

    float MaxValue = 0.f;

    std::function<void(EventQueue&, float)> OnValueChange;

    std::atomic<float> Fraction{0.f};
  };

  static void PressAt(const std::shared_ptr<SliderState>& State,
                      EventQueue& Queue, float MouseX);
  static void PushFill(const SliderState& State, float Fraction);

  std::shared_ptr<SliderState> State;
};
