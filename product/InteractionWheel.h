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
  Ogre::Vector2f LatLonR;
  Ogre::Vector3f Position;
  void ShareInfoSelectedEntReceive(NotifySelectedEntity Event);
  void ShareInfoLatLonReceive(NotifyLatLonEvent Event);
  void ShareInfoHitPosReceive(NotifyPosEvent Event);
  void CallBackButtonA(CallBackACommand Cmd);
  void CallBackButtonB(CallBackBCommand Cmd);
  void CallBackButtonC(CallBackCCommand Cmd);
  void CallBackButtonD(CallBackDCommand Cmd);
  bool Visibility;

  int ThreadID;
};
