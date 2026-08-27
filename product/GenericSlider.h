// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "InputDevice.h"
#include "RenderSystem.h"
#include "UIElement.h"

// a draggable track whose fill follows the value it holds. LabelText is drawn
// above the track and is the only thing that says what the value means, so it
// is set per slider rather than baked in
//
// the track spans MinSliderValue to MaxSliderValue rather than always starting
// at zero, because plenty of the things a slider drives have a floor that is
// not zero - a window 0 pixels wide, or a globe of 0 subdivisions, is not a
// setting anybody wants to be one drag away from
class GenericSlider : public UIElement {
 public:
  GenericSlider(std::string SliderName, std::string LabelText,
                std::vector<float> Position, std::vector<float> Dimensions,
                float MinSliderValue, float MaxSliderValue,
                InputDevice* DeviceToRespondTo,
                std::function<void(EventQueue&, float)> ChangeCallback,
                EventQueue* QueueForCallBack, int ThreadId);

  void ChangeVisibility(bool Visible) override;
  void MaintainScaling() override;

  float GetValue() const;
  void SetValue(float NewValue);

  // replace the text drawn above the track. a slider that is standing in for
  // a number has to be able to say which number, and that changes as it is
  // dragged, so the label cannot be fixed at construction
  void SetLabelText(std::string NewText) const;

 private:
  struct SliderState {
    std::string TrackName;
    std::string FillName;
    std::string LabelName;
    std::string OverlayName;

    std::vector<float> Pos{0.f, 0.f};
    std::vector<float> Size{0.f, 0.f};

    float MinValue = 0.f;
    float MaxValue = 0.f;

    std::function<void(EventQueue&, float)> OnValueChange;

    std::atomic<float> Fraction{0.f};
  };

  static void PressAt(const std::shared_ptr<SliderState>& State,
                      EventQueue& Queue, float MouseX);
  static void PushFill(const SliderState& State, float Fraction);
  static float ValueForFraction(const SliderState& State, float Fraction);

  std::shared_ptr<SliderState> State;
};
