// Copyright (c) 2025 Henry Frodsham
#pragma once

struct ChangeCursorVisibilityEvent {
  bool Visibile;
  explicit ChangeCursorVisibilityEvent(bool V) : Visibile(V) {}
};
