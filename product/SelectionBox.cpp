// Copyright (c) 2026 Henry Frodsham
#include "SelectionBox.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {
// how thick the outline is drawn, as a fraction of the viewport. the same
// number is used on both axes: an overlay's relative space is not square, so
// the vertical edges come out a little thinner than the horizontal ones, and
// at a line this thin nobody can tell
constexpr float kEdgeThickness = 0.003f;
}  // namespace

SelectionBox::SelectionBox(std::string BoxName, InputDevice* DeviceToRespondTo,
                           std::string EdgeMaterial, int ThreadId) {
  const std::string Suffix = "_" + std::to_string(ThreadId);

  // its own overlay rather than the shared UI_Overlay_<id>, so the band can be
  // shown and hidden on every drag without touching anything else that is
  // parked on that overlay
  OverlayName = "SELECTION_BOX_OVERLAY" + Suffix;

  EdgeNames = {BoxName + "_top" + Suffix, BoxName + "_bottom" + Suffix,
               BoxName + "_left" + Suffix, BoxName + "_right" + Suffix};

  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(CreateOverlayEvent(OverlayName, DeviceToRespondTo));

  for (const std::string& EdgeName : EdgeNames) {
    RS.RenderQueue->Enqueue(OverlayAddBoxEvent({0.f, 0.f}, {0.f, 0.f}, EdgeName,
                                               EdgeMaterial, OverlayName));
    RS.RenderQueue->Enqueue(
        ChangeOverlayVisibilityEvent(OverlayName, EdgeName, false));
  }
}

void SelectionBox::ChangeVisibility(bool ShouldBeVisible) {
  // a drag publishes an update on every cursor move, and every one of those
  // would otherwise re-send four visibility events for a band that is already
  // on screen
  if (Visible == ShouldBeVisible) {
    return;
  }
  Visible = ShouldBeVisible;

  RenderSystem& RS = RenderSystem::GetInstance();
  for (const std::string& EdgeName : EdgeNames) {
    RS.RenderQueue->Enqueue(
        ChangeOverlayVisibilityEvent(OverlayName, EdgeName, ShouldBeVisible));
  }
}

void SelectionBox::SetCorners(float OriginX, float OriginY, float CornerX,
                              float CornerY) {
  const float Left = std::min(OriginX, CornerX);
  const float Right = std::max(OriginX, CornerX);
  const float Top = std::min(OriginY, CornerY);
  const float Bottom = std::max(OriginY, CornerY);

  const float Width = Right - Left;
  const float Height = Bottom - Top;

  // horizontal edges run the full width, vertical ones are inset by that
  // thickness at each end so the corners meet instead of overlapping
  PlaceEdge(EdgeNames[0], Left, Top, Width, kEdgeThickness);
  PlaceEdge(EdgeNames[1], Left, Bottom - kEdgeThickness, Width, kEdgeThickness);
  PlaceEdge(EdgeNames[2], Left, Top, kEdgeThickness, Height);
  PlaceEdge(EdgeNames[3], Right - kEdgeThickness, Top, kEdgeThickness, Height);
}

void SelectionBox::PlaceEdge(const std::string& EdgeName, float Left, float Top,
                             float Width, float Height) const {
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      EdgeName, OverlayName, {std::max(Width, 0.f), std::max(Height, 0.f)},
      {Left, Top}));
}
