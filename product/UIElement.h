// Copyright (c) 2026 Henry Frodsham
#pragma once

// common interface for anything a state can own and display, so a state can
// show, hide and rescale its UI without knowing what each element actually is
class UIElement {
 public:
  virtual ~UIElement() = default;

  virtual void ChangeVisibility(bool Visible) = 0;
  virtual void MaintainScaling() = 0;
};
