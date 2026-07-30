// Copyright (c) 2026 Henry Frodsham
#pragma once

struct ChangeGlobeVisibilityEvent {
  bool Visible;
  explicit ChangeGlobeVisibilityEvent(bool Vis) : Visible(Vis) {}
};