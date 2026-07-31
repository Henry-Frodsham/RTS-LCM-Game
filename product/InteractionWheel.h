// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "ECSHelper.h"
#include "InputTranslator.h"
#include "InteractionWheelCallbackCommands.h"
#include "RenderSystem.h"
#include "ShareInfoEvent.h"
#include "UIEvent.h"

// the quick interaction wheel accessible through RT or right click
class InteractionWheel {
 public:
  InteractionWheel(InputTranslator* Device, int ThreadNum,
                   ECSHelper* Interactor, Player* Play);

  void UpdateAndWarmupContext();

  void OnContextActionCommand(ContextActionCommand Cmd);

  void OnPressActionCommand(PressActionCommand Cmd);

  EventQueue* ForeignNotifQueue;
  ECSHelper* Factory;

  Player* GamePlayer;

 private:
  InputTranslator* DeviceState;

  EventBus* ForeignNotifBus;
  Ogre::Entity* SelectedEntity;
  Ogre::Vector3f Position;
  Ogre::Vector3f SurfaceNormal;

  void ShareInfoSelectedEntReceive(NotifySelectedEntity Event);
  void ShareInfoHitPosReceive(NotifyPosEvent Event);
  void ShareInfoSurfaceNormal(NotifySurfaceNormalEvent Event);
  void CallBackButtonA(CallBackACommand Cmd);
  void CallBackButtonB(CallBackBCommand Cmd);
  void CallBackButtonC(CallBackCCommand Cmd);
  void CallBackButtonD(CallBackDCommand Cmd);
  bool Visibility;

  int ThreadID;
};
