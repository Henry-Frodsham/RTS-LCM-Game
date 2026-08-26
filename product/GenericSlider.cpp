// Copyright (c) 2026 Henry Frodsham
#include "GenericSlider.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

GenericSlider::GenericSlider(
    std::string SliderName, std::vector<float> Position,
    std::vector<float> Dimensions, float MaxSliderValue,
    InputDevice* DeviceToRespondTo,
    std::function<void(EventQueue&, float)> ChangeCallback,
    EventQueue* QueueForCallBack, int ThreadId)
    : State(std::make_shared<SliderState>()) {
  const std::string Suffix = "_" + std::to_string(ThreadId);

  State->TrackName = SliderName + Suffix;
  State->FillName = SliderName + "_fill" + Suffix;
  State->OverlayName = "UI_Overlay_" + std::to_string(ThreadId);
  State->MaxValue = MaxSliderValue;
  State->OnValueChange = std::move(ChangeCallback);

  if (Position.size() >= 2) {
    State->Pos = Position;
  }
  if (Dimensions.size() >= 2) {
    State->Size = Dimensions;
  }

  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(
      CreateOverlayEvent(State->OverlayName, DeviceToRespondTo));

  RS.RenderQueue->Enqueue(OverlayAddBoxEvent(State->Pos, State->Size,
                                             State->TrackName, "BLACK",
                                             State->OverlayName));

  RS.RenderQueue->Enqueue(
      OverlayAddBoxToPanelEvent(State->TrackName, State->FillName, "RED",
                                {0.f, 0.f}, {0.f, State->Size[1]}));

  std::shared_ptr<SliderState> Captured = State;
  RS.RenderQueue->Enqueue(RegisterOnPressCallBackEvent(
      State->OverlayName, State->TrackName,
      [Captured](EventQueue& Queue, float MouseX, float MouseY) {
        PressAt(Captured, Queue, MouseX);
      },
      QueueForCallBack, false));
}

void GenericSlider::ChangeVisibility(bool Visible) {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      State->OverlayName, State->TrackName, Visible));

  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      State->OverlayName, State->FillName, Visible));
}

void GenericSlider::MaintainScaling() {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      State->TrackName, State->OverlayName, State->Size, State->Pos,
      "USE_OLD"));

  PushFill(*State, State->Fraction.load(std::memory_order_relaxed));
}

float GenericSlider::GetValue() const {
  return State->Fraction.load(std::memory_order_relaxed) * State->MaxValue;
}

void GenericSlider::SetValue(float NewValue) {
  const float Fraction =
      (State->MaxValue > 0.f)
          ? std::clamp(NewValue / State->MaxValue, 0.f, 1.f)
          : 0.f;

  State->Fraction.store(Fraction, std::memory_order_relaxed);
  PushFill(*State, Fraction);
}

void GenericSlider::PressAt(const std::shared_ptr<SliderState>& State,
                            EventQueue& Queue, float MouseX) {
  const float TrackWidth = State->Size[0];
  const float Fraction =
      (TrackWidth > 0.f)
          ? std::clamp((MouseX - State->Pos[0]) / TrackWidth, 0.f, 1.f)
          : 0.f;

  State->Fraction.store(Fraction, std::memory_order_relaxed);
  PushFill(*State, Fraction);

  if (State->OnValueChange) {
    State->OnValueChange(Queue, Fraction * State->MaxValue);
  }
}

void GenericSlider::PushFill(const SliderState& State, float Fraction) {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      State.FillName, State.OverlayName,
      {Fraction * State.Size[0], State.Size[1]}, {-1.f, -1.f}, "USE_OLD"));
}
