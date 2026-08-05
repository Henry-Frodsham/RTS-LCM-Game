// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "ECSHelper.h"
#include "InputTranslator.h"
#include "InteractionWheelCallbackCommands.h"
#include "RenderSystem.h"
#include "ShareInfoEvent.h"
#include "UIEvent.h"
#include "Biome.h"
#include <entt/entt.hpp>

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
  entt::entity SelectedEntity;
  Ogre::Vector3f Position;
  Ogre::Vector3f SurfaceNormal;

  BiomeType SelectedBiome;

  void ShareInfoSelectedEntReceive(NotifyEntityResult Event);
  void SucessfulEntitySelection(SelectEntitySuccessEvent Event);
  void ReceiveRayResult(NotifyRayResult Event);
  void CallBackButtonA(CallBackACommand Cmd);
  void CallBackButtonB(CallBackBCommand Cmd);
  void CallBackButtonC(CallBackCCommand Cmd);
  void CallBackButtonD(CallBackDCommand Cmd);
  bool Visibility;

  int ThreadID;
};
