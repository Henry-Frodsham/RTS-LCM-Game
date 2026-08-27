// Copyright (c) 2026 Henry Frodsham
#pragma once
#include "BaseOptionState.h"
#include "InputDevice.h"

// the options page belonging to a single split screen instance, reached from
// that instance's InstanceMenuState. MainOptionState stays the keyboard and
// mouse page and keeps the settings that are the whole application's, this one
// only carries settings that are per player
//
// so it is built with its instance number as the config instance name, and
// every setting bound here writes that player's own file - the same one their
// InputTranslator was built from
class InstanceOptionState : public BaseOptionState {
 public:
  InstanceOptionState(EventQueue* CallBacksQueue, InputDevice* InstanceDevice,
                      int InstanceId);

 protected:
  void OnBindSettings() override;
};
