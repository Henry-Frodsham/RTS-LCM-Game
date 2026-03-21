#pragma once
#include "InputTranslator.h"
#include "EventQueue.h"
#include "PlayerControlEvent.h"
#include "RenderSystem.h"
#include "RayTraceEvent.h"
#include "InteractionWheel.h"
#include "ShareInfoEvent.h"

class PlayerGeneralControl {
 public:
  PlayerGeneralControl(InputTranslator* Translator, EventQueue* Queue, InteractionWheel* Wheel);
  InputTranslator* PlayerTranslator;

  EventQueue* ControlQueue;
  InteractionWheel* InteractionWheelToNotify;
  void Update(float Dt);
  void OnPress(PressActionCommand Cmd);
  void OnCompletedTrace(EndRayTraceResultEvent Event);
  EventBus* TriggerBus;
 private:
  EventQueue* TriggerQueue;

  Ogre::Vector2f HitPointToDeltaLatLon(Ogre::Vector3 UnitPos,
                                       Ogre::Vector3 HitPoint);

  Ogre::Vector2f LastDeltaLatLon;
  Ogre::Entity* SelectedEntity;
};