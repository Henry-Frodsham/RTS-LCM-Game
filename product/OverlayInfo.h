// Copyright © 2025 Henry Frodsham
#pragma once

struct OverlayInfo {
  bool Pressed;
  bool Hovered;
  std::string OwnedByOverlay;
  OverlayInfo(bool Press, bool Hvr, std::string OwnedOverlay)
      : Pressed(Press), Hovered(Hvr), OwnedByOverlay(OwnedOverlay) {}
};
