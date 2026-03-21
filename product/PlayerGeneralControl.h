#pragma once
#include "InputTranslator.h"
#include "EventQueue.h"
#include "PlayerControlEvent.h"
#include "RenderSystem.h"
#include "RayTraceEvent.h"

class PlayerGeneralControl {
 public:
  PlayerGeneralControl(InputTranslator* Translator, EventQueue* Queue);
  InputTranslator* PlayerTranslator;

  EventQueue* ControlQueue;

  void Update(float Dt);
  void OnPress(PressActionCommand Cmd);
  void OnCompletedTrace(EndRayTraceResultEvent Event);
  EventBus* TriggerBus;
 private:
  EventQueue* TriggerQueue;
};