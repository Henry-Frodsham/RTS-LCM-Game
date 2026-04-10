// Copyright (c) 2025 Henry Frodsham
#pragma once

#include <string>
// struct associated with an overlay, tracking if the overlay is hovered and who
// owns it
struct OverlayInfo {
  bool Pressed;
  bool Hovered;
  std::string OwnedByOverlay;

  std::function<void(EventQueue&)> PressCallBack;

  EventQueue* CallQueue;
  OverlayInfo(bool Press, bool Hvr, std::string OwnedOverlay)
      : Pressed(Press),
        Hovered(Hvr),
        OwnedByOverlay(OwnedOverlay),
        PressCallBack(nullptr),
        CallQueue(nullptr) {}
};
