// Copyright (c) 2025 Henry Frodsham
#include "GenericButton.h"

#include <string>
#include <utility>
#include <vector>

namespace {
// clearance left between the caption and the edge of the box. x is a fraction
// of the viewport's width and y of its height, so equal values already read as
// the wider horizontal margin a button wants
constexpr float PaddingX = 0.008f;
constexpr float PaddingY = 0.008f;
}  // namespace

// generic button class
// helper class that automatically defines a button with a specified behaviour
GenericButton::GenericButton(
    std::string ButtonName, std::string ButtonText, std::vector<float> Position,
    InputDevice* DeviceToRespondTo,
    std::function<void(EventQueue&, float, float)> PressCallback,
    EventQueue* QueueForCallback, int ThreadID) {
  const std::string Suffix = "_" + std::to_string(ThreadID);

  BoxName = ButtonName + Suffix;
  TextName = ButtonName + "_text" + Suffix;
  OverlayName = "UI_Overlay_" + std::to_string(ThreadID);

  if (Position.size() >= 2) {
    Centre = Position;
  }

  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(CreateOverlayEvent(OverlayName, DeviceToRespondTo));

  // the box starts empty because only the render side can measure the caption
  // it has to wrap around - FitToText gives it its real size and position
  RS.RenderQueue->Enqueue(
      OverlayAddBoxEvent(Centre, {0.f, 0.f}, BoxName, "RED", OverlayName));

  RS.RenderQueue->Enqueue(OverlayAddTextToPanelEvent{
      BoxName, TextName, ButtonText, "WHITE", {0.f, 0.f}, {1.f, 1.f}});

  RS.RenderQueue->Enqueue(RegisterOnPressCallBackEvent(
      OverlayName, BoxName, std::move(PressCallback), QueueForCallback));

  FitToText();
}

void GenericButton::ChangeVisibility(bool Visible) {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(
      ChangeOverlayVisibilityEvent(OverlayName, BoxName, Visible));

  RS.RenderQueue->Enqueue(
      ChangeOverlayVisibilityEvent(OverlayName, TextName, Visible));
}

void GenericButton::SetText(std::string NewText) const {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(OverlayEditTextEvent(TextName, OverlayName,
                                               {-1.f, -1.f}, {-1.f, -1.f},
                                               "USE_OLD", std::move(NewText)));

  // the old caption is what the box was sized around, so it has to be fitted
  // again rather than left wrapping text that is no longer there
  FitToText();
}

// the caption's width depends on the viewport's aspect, so the fit has to be
// redone rather than kept, exactly like any other rescaled element
void GenericButton::MaintainScaling() { FitToText(); }

void GenericButton::FitToText() const {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(OverlayFitPanelToTextEvent(
      BoxName, TextName, OverlayName, Centre, {PaddingX, PaddingY}));
}
