#pragma once
#include "InputTranslator.h"
#include "RenderSystem.h"
#include "UIEvent.h"
#include "ShareInfoEvent.h"
#include "InteractionWheelCallbackCommands.h"
#include "ECSHelper.h"

// the quick interaction wheel accessible through RT or right click
class InteractionWheel {
 public:
  InteractionWheel(InputTranslator* Device, int ThreadNum, ECSHelper* Interactor);

  void UpdateAndWarmupContext();

  void OnContextActionCommand(ContextActionCommand Cmd);

  void OnPressActionCommand(PressActionCommand Cmd);

  
  EventQueue* ForeignNotifQueue;
  ECSHelper* Factory;
 private:
  InputTranslator* DeviceState;

  EventBus* ForeignNotifBus;
  Ogre::Entity* SelectedEntity;
  Ogre::Vector2f LatLonR;
  void ShareInfoSelectedEntReceive(NotifySelectedEntity Event);
  void ShareInfoLatLonReceive(NotifyLatLonEvent Event);

  void CallBackButtonA(CallBackACommand Cmd);
  void CallBackButtonB(CallBackBCommand Cmd);
  void CallBackButtonC(CallBackCCommand Cmd);
  void CallBackButtonD(CallBackDCommand Cmd);
  bool Visibility;

  int ThreadID;
};
