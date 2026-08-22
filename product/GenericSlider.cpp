// Copyright (c) 2026 Henry Frodsham
#include <algorithm>
#include <string>
#include <vector>

#include "GenericSlider.h"
// generic slider class
// helper class that automatically defines a slider with a specified
// behaviour - a black track with a red fill on top that scales to show the
// pressed position, like a slider handle
GenericSlider::GenericSlider(std::string ButtonName, std::string ButtonText,
                             std::vector<float> Position,
                             std::vector<float> SliderSize, float MaxSliderValue,
                             float StepPerPixel,
                             InputDevice* DeviceToRespondTo,
                             std::function<void(EventQueue&, float, float)> ChangeCallback,
                             EventQueue* QueueForCallback, int ThreadID)
    : Name(ButtonName),
      Text(ButtonText),
      Pos(Position),
      Size(SliderSize),
      MaxValue(MaxSliderValue),
      StepValue(StepPerPixel),
      Device(DeviceToRespondTo),
      CallbackOnValueChange(ChangeCallback),
      CallBackQueue(QueueForCallback),

      Id(ThreadID) {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(
      CreateOverlayEvent("UI_Overlay_" + std::to_string(Id), Device));

  RS.RenderQueue->Enqueue(
      OverlayAddBoxEvent(Pos, Size, Name + "_" + std::to_string(Id),
                         "BLACK", "UI_Overlay_" + std::to_string(ThreadID)));

  // the fill is a child of the track panel rather than its own top-level
  // overlay element, so it renders on top of the track without being
  // separately hover/press-checked by OverlayController (same convention as
  // AddTextToPanel for button text). despite being a child, Ogre's
  // GMM_RELATIVE dimensions are still fractions of the whole overlay/screen,
  // not of the parent panel - only position is parent-relative - so the fill
  // has to be sized in Size units to actually match the track, not [0,1].
  // starts at zero width until the first press gives it a value
  RS.RenderQueue->Enqueue(OverlayAddBoxToPanelEvent(
      Name + "_" + std::to_string(Id), Name + "_fill_" + std::to_string(Id),
      "RED", {0.f, 0.f}, {0.f, Size[1]}));

  RS.RenderQueue->Enqueue(RegisterOnPressCallBackEvent(
      "UI_Overlay_" + std::to_string(ThreadID), Name + "_" + std::to_string(Id),
      [this](EventQueue& Queue, float MouseX, float MouseY) {
        HandlePress(Queue, MouseX, MouseY);
      },
      CallBackQueue));
}

void GenericSlider::ChangeVisibility(bool Visible) {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(
      ChangeOverlayVisibilityEvent("UI_Overlay_" + std::to_string(Id),
                                   Name + "_" + std::to_string(Id), Visible));

  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(Id), Name + "_fill_" + std::to_string(Id),
      Visible));
}

void GenericSlider::MaintainScaling() {
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      Name + "_" + std::to_string(Id), "UI_Overlay_" + std::to_string(Id),
      Size, {-1.f, -1.f}, "USE_OLD"));

  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      Name + "_fill_" + std::to_string(Id), "UI_Overlay_" + std::to_string(Id),
      {FillFraction() * Size[0], Size[1]}, {-1.f, -1.f}, "USE_OLD"));
}

float GenericSlider::FillFraction() const {
  if (MaxValue <= 0.f) {
    return 0.f;
  }
  return std::clamp(Value / MaxValue, 0.f, 1.f);
}

void GenericSlider::HandlePress(EventQueue& Queue, float MouseX,
                                float MouseY) {
  // Pos/MouseX are both in normalised 0-1 viewport space, but StepValue is
  // defined per screen pixel, so convert the press's offset from the track's
  // left edge into pixels before applying it
  RenderSystem& RS = RenderSystem::GetInstance();
  float WindowWidth = RS.GetRenderWindowDimensions()[0];

  float PixelOffset = (MouseX - Pos[0]) * WindowWidth;
  Value = std::clamp(PixelOffset * StepValue, 0.f, MaxValue);

  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      Name + "_fill_" + std::to_string(Id), "UI_Overlay_" + std::to_string(Id),
      {FillFraction() * Size[0], Size[1]}, {-1.f, -1.f}, "USE_OLD"));

  if (CallbackOnValueChange) {
    CallbackOnValueChange(Queue, MouseX, MouseY);
  }
}
