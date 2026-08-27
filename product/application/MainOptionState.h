// Copyright (c) 2026 Henry Frodsham
#pragma once
#include "BaseOptionState.h"
#include "GlobeEvent.h"
#include "RenderSystem.h"

// the keyboard and mouse options page, and the one the whole application's
// settings belong on - anything that is not a single player's. its configs are
// the shared ones, so it is built with no instance name; a setting that really
// is per player is reached with Config("InputSettings", "1") instead
//
// world generation has a tab of its own, reached from here, because those
// settings are read once when the globe is built rather than applied where
// they stand - see GlobeOptionState
class MainOptionState : public BaseOptionState {
 public:
  explicit MainOptionState(EventQueue* CallBacksQueue);

 protected:
  void OnBindSettings() override;
  void OnPageEnter() override;

 private:
  
};
