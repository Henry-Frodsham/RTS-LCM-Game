// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "InputDevice.h"
#include "RenderSystem.h"
#include "UIElement.h"

// a labelled box that runs a callback when it is pressed. the box is never
// given a size of its own - it is sized around its caption and centred on the
// position it was built with, so a longer label widens the button instead of
// spilling out of it
class GenericButton : public UIElement {
 public:
  GenericButton(std::string ButtonName, std::string ButtonText,
                std::vector<float> Position, InputDevice* DeviceToRespondTo,
                std::function<void(EventQueue&, float, float)> PressCallback,
                EventQueue* QueueForCallBack, int ThreadId);
  void ChangeVisibility(bool Visible) override;
  void MaintainScaling() override;

  // replace the caption, and let the box resize around it. a button whose
  // label is the value it holds - a setting that is on or off - has no other
  // way to show that the value changed
  void SetText(std::string NewText) const;

 private:
  void FitToText() const;

  std::string BoxName;
  std::string TextName;
  std::string OverlayName;

  std::vector<float> Centre{0.f, 0.f};
};
