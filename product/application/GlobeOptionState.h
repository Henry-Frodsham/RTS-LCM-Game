// Copyright (c) 2026 Henry Frodsham
#pragma once
#include "BaseOptionState.h"
#include "GlobeEvent.h"
#include "RenderSystem.h"

// the world generation tab, opened from the keyboard and mouse options page
// and belonging to that instance alone - a controller player has no say in the
// shape of the shared world, so InstanceOptionState carries none of this
//
// everything bound here is read once, when the globe is built, which is when
// play is pressed. so a change made here is not something that can be applied
// to the world already on screen - it is what the next one is made from
class GlobeOptionState : public BaseOptionState {
 public:
  explicit GlobeOptionState(EventQueue* CallBacksQueue);

 protected:
  void OnBindSettings() override;
  void OnPageEnter() override;

  // a tab of the main options page rather than a page of its own, so back
  // leads to the page it was opened from
  AppState GetBackState() const override { return AppState::MENU; }
};
