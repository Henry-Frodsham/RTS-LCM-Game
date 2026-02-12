#pragma once
#include "InputTranslator.h"
#include "RenderSystem.h"
#include "UIEvent.h"

// the quick interaction wheel accessible through RT or right click
class InteractionWheel {
 public:
  InteractionWheel(InputTranslator* Device, EventBus* Bus, int ThreadNum);

  void ToggleVisibility(ToggleInteractionWheelEvent Event);

 private:
  InputTranslator* DeviceState;

  EventBus* IssuingBus;

  int ThreadID;
};
