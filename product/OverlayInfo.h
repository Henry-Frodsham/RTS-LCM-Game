// Copyright (c) 2025 Henry Frodsham
#pragma once

#include <string>
// struct associated with an overlay, tracking if the overlay is hovered and who
// owns it
struct OverlayInfo {
  bool Pressed;
  bool Hovered;
  std::string OwnedByOverlay;

  std::function<void(EventQueue&, float, float)> PressCallBack;

  EventQueue* CallQueue;

  // material to restore on release - defaults to "RED" to match the
  // long-standing button convention (idle red/hover blue/press purple) for
  // any element that doesn't specify its own, e.g. GenericButton's track
  std::string BaseMaterial;
  OverlayInfo(bool Press, bool Hvr, std::string OwnedOverlay,
             std::string Base = "RED")
      : Pressed(Press),
        Hovered(Hvr),
        OwnedByOverlay(OwnedOverlay),
        PressCallBack(nullptr),
        CallQueue(nullptr),
        BaseMaterial(Base) {}
};
