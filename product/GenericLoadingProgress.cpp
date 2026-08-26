// Copyright (c) 2026 Henry Frodsham
#include "GenericLoadingProgress.h"

#include <algorithm>
#include <string>
#include <vector>

GenericLoadingProgress::GenericLoadingProgress(
    std::string BarName, std::vector<float> Position,
    std::vector<float> Dimensions, InputDevice* DeviceToAttachTo,
    int ThreadId) {
  const std::string Suffix = "_" + std::to_string(ThreadId);

  TrackName = BarName + Suffix;
  FillName = BarName + "_fill" + Suffix;
  OverlayName = "UI_Overlay_" + std::to_string(ThreadId);

  if (Position.size() >= 2) {
    Pos = Position;
  }
  if (Dimensions.size() >= 2) {
    Size = Dimensions;
  }

  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(CreateOverlayEvent(OverlayName, DeviceToAttachTo));

  RS.RenderQueue->Enqueue(
      OverlayAddBoxEvent(Pos, Size, TrackName, "BLACK", OverlayName));

  RS.RenderQueue->Enqueue(OverlayAddBoxToPanelEvent(
      TrackName, FillName, "RED", {0.f, 0.f}, {0.f, Size[1]}));
}

void GenericLoadingProgress::ChangeVisibility(bool Visible) {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(
      ChangeOverlayVisibilityEvent(OverlayName, TrackName, Visible));

  RS.RenderQueue->Enqueue(
      ChangeOverlayVisibilityEvent(OverlayName, FillName, Visible));
}

void GenericLoadingProgress::MaintainScaling() {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(
      OverlayEditPanelEvent(TrackName, OverlayName, Size, Pos, "USE_OLD"));

  PushFill(Progress.load(std::memory_order_relaxed));
}

void GenericLoadingProgress::SetProgress(float Fraction) {
  const float Clamped = std::clamp(Fraction, 0.f, 1.f);

  Progress.store(Clamped, std::memory_order_relaxed);
  PushFill(Clamped);
}

float GenericLoadingProgress::GetProgress() const {
  return Progress.load(std::memory_order_relaxed);
}

void GenericLoadingProgress::PushFill(float Fraction) const {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(FillName, OverlayName,
                                                {Fraction * Size[0], Size[1]},
                                                {-1.f, -1.f}, "USE_OLD"));
}
