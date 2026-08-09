// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <entt/entt.hpp>

#include "EventQueue.h"
#include "InputTranslator.h"
#include "InteractionWheel.h"
#include "PlayerControlEvent.h"
#include "RayTraceEvent.h"
#include "RenderSystem.h"
#include "ShareInfoEvent.h"
// class responsible for general player control over the game world
// such as (but not limited to), rotating the view point
// checking hit points
class PlayerGeneralControl {
 public:
  PlayerGeneralControl(InputTranslator* Translator, EventQueue* Queue,
                       InteractionWheel* Wheel);
  InputTranslator* PlayerTranslator;

  EventQueue* ControlQueue;
  InteractionWheel* InteractionWheelToNotify;
  void Update(float Dt);
  void OnPress(PressActionCommand Cmd);
  void OnCompletedTrace(RayPickResult Event);
  EventBus* TriggerBus;

 private:
  EventQueue* TriggerQueue;

  Ogre::Entity* SelectedEntity;

  // issues a fresh raytrace at the current cursor position, used every
  // frame while hold-to-preview is active (OnPress only fires once, on click)
  void TriggerPreviewRayTrace();

  bool WasPreviewing = false;
};
