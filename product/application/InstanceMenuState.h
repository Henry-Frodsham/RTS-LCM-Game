// Copyright (c) 2026 Henry Frodsham
#pragma once
#include "BaseState.h"
#include "GenericButton.h"
#include "InputDevice.h"

// the menu a single split screen instance sees in place of the main menu. only
// a controller instance gets one - the keyboard and mouse instance (thread 1)
// keeps MenuState, because it is the one that actually starts the game - so
// where the main menu offers play, this offers ready
class InstanceMenuState : public BaseState {
 public:
  InstanceMenuState(EventQueue* CallBacksQueue, InputDevice* InstanceDevice,
                    int InstanceId);

 protected:
  void OnInit() override;

 private:
  // the instance this page belongs to. every element is built against both, so
  // the page is drawn in, and only reacts to, that instance's viewport
  InputDevice* Device;
  int Id;

  GenericButton* ReadyButton;
  GenericButton* OptionButton;

};
