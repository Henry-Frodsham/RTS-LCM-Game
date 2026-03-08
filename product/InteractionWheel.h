#pragma once
#include "InputTranslator.h"
#include "RenderSystem.h"
#include "UIEvent.h"

// the quick interaction wheel accessible through RT or right click
class InteractionWheel {
 public:
  InteractionWheel(InputTranslator* Device, int ThreadNum);

  void UpdateAndWarmupContext();

  void OnContextActionCommand(ContextActionCommand Cmd);

  void OnPressActionCommand(PressActionCommand Cmd);

 private:
  InputTranslator* DeviceState;

  bool Visibility;

  int ThreadID;
};
