// Copyright (c) 2026 Henry Frodsham
#include "GenericSlider.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {
// how far above the track's top edge the label's own top sits - a character
// height plus a little clear space, so the two never touch
constexpr float LabelOffset = 0.033f;
}  // namespace

GenericSlider::GenericSlider(
    std::string SliderName, std::string LabelText, std::vector<float> Position,
    std::vector<float> Dimensions, float MinSliderValue, float MaxSliderValue,
    InputDevice* DeviceToRespondTo,
    std::function<void(EventQueue&, float)> ChangeCallback,
    EventQueue* QueueForCallBack, int ThreadId)
    : State(std::make_shared<SliderState>()) {
  const std::string Suffix = "_" + std::to_string(ThreadId);

  State->TrackName = SliderName + Suffix;
  State->FillName = SliderName + "_fill" + Suffix;
  State->LabelName = SliderName + "_label" + Suffix;
  State->OverlayName = "UI_Overlay_" + std::to_string(ThreadId);
  State->MinValue = MinSliderValue;
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

  // a child is offset from its parent's corner, so a negative y lifts the
  // label clear of the track it names
  RS.RenderQueue->Enqueue(OverlayAddTextToPanelEvent{State->TrackName,
                                                     State->LabelName,
                                                     LabelText,
                                                     "WHITE",
                                                     {0.f, -LabelOffset},
                                                     State->Size});

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

  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      State->OverlayName, State->LabelName, Visible));
}

void GenericSlider::MaintainScaling() {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      State->TrackName, State->OverlayName, State->Size, State->Pos,
      "USE_OLD"));

  PushFill(*State, State->Fraction.load(std::memory_order_relaxed));
}

float GenericSlider::GetValue() const {
  return ValueForFraction(*State,
                          State->Fraction.load(std::memory_order_relaxed));
}

void GenericSlider::SetValue(float NewValue) {
  const float Span = State->MaxValue - State->MinValue;
  const float Fraction =
      (Span > 0.f) ? std::clamp((NewValue - State->MinValue) / Span, 0.f, 1.f)
                   : 0.f;

  State->Fraction.store(Fraction, std::memory_order_relaxed);
  PushFill(*State, Fraction);
}

void GenericSlider::SetLabelText(std::string NewText) const {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(OverlayEditTextEvent(
      State->LabelName, State->OverlayName, {-1.f, -1.f}, {-1.f, -1.f},
      "USE_OLD", std::move(NewText)));
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
    State->OnValueChange(Queue, ValueForFraction(*State, Fraction));
  }
}

// a fraction of the way along the track is a value somewhere between the two
// ends of the range, not a fraction of the top one
float GenericSlider::ValueForFraction(const SliderState& State,
                                      float Fraction) {
  return State.MinValue + (Fraction * (State.MaxValue - State.MinValue));
}

void GenericSlider::PushFill(const SliderState& State, float Fraction) {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      State.FillName, State.OverlayName,
      {Fraction * State.Size[0], State.Size[1]}, {-1.f, -1.f}, "USE_OLD"));
}
