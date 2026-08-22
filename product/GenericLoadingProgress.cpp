// Copyright (c) 2026 Henry Frodsham
#include <algorithm>
#include <string>
#include <vector>

#include "GenericLoadingProgress.h"
// generic loading progress bar class
// same track+fill visual as GenericSlider (a black track with a red fill
// that scales to show progress), but the fill is driven by SetProgress()
// rather than a press, so it never registers an input callback
GenericLoadingProgress::GenericLoadingProgress(std::string BarName,
                                               std::string BarText,
                                               std::vector<float> Position,
                                               InputDevice* DeviceToAttachTo,
                                               int ThreadID)
    : Name(BarName),
      Text(BarText),
      Pos(Position),
      Device(DeviceToAttachTo),
      Id(ThreadID) {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(
      CreateOverlayEvent("UI_Overlay_" + std::to_string(Id), Device));

  RS.RenderQueue->Enqueue(
      OverlayAddBoxEvent(Pos, {kSize, kSize}, Name + "_" + std::to_string(Id),
                         "BLACK", "UI_Overlay_" + std::to_string(ThreadID)));

  // fill is a child of the track panel, same convention as GenericSlider -
  // starts at zero width until SetProgress is first called
  RS.RenderQueue->Enqueue(OverlayAddBoxToPanelEvent(
      Name + "_" + std::to_string(Id), Name + "_fill_" + std::to_string(Id),
      "RED", {0.f, 0.f}, {0.f, 1.f}));
}

void GenericLoadingProgress::ChangeVisibility(bool Visible) {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(
      ChangeOverlayVisibilityEvent("UI_Overlay_" + std::to_string(Id),
                                   Name + "_" + std::to_string(Id), Visible));

  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(Id), Name + "_fill_" + std::to_string(Id),
      Visible));
}

void GenericLoadingProgress::MaintainScaling() {
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      Name + "_" + std::to_string(Id), "UI_Overlay_" + std::to_string(Id),
      {kSize, kSize}, {-1.f, -1.f}, "USE_OLD"));

  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      Name + "_fill_" + std::to_string(Id), "UI_Overlay_" + std::to_string(Id),
      {Value, 1.f}, {-1.f, -1.f}, "USE_OLD"));
}

void GenericLoadingProgress::SetProgress(float Fraction) {
  Value = std::clamp(Fraction, 0.f, 1.f);

  RenderSystem& RS = RenderSystem::GetInstance();
  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      Name + "_fill_" + std::to_string(Id), "UI_Overlay_" + std::to_string(Id),
      {Value, 1.f}, {-1.f, -1.f}, "USE_OLD"));
}
