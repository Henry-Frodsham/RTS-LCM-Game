// Copyright © 2025 Henry Frodsham
#pragma once

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
