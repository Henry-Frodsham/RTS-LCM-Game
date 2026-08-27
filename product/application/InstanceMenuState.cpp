// Copyright (c) 2026 Henry Frodsham
#include "InstanceMenuState.h"

#include <vector>

InstanceMenuState::InstanceMenuState(EventQueue* CallBacksQueue,
                                     InputDevice* InstanceDevice,
                                     int InstanceId)
    : BaseState(CallBacksQueue, AppState::MENU, InstanceId),
      Device(InstanceDevice),
      Id(InstanceId),
      ReadyButton(nullptr),
      OptionButton(nullptr) {}

void InstanceMenuState::OnInit() {
  // readying up is what will eventually decide when the shared game may start,
  // so the press does nothing until that handshake exists
  ReadyButton = CreateElement<GenericButton>(
      "instance_menu_ready", "ready", std::vector<float>{0.5f, 0.5f}, Device,
      [](EventQueue& Queue, float MouseX, float MouseY) {}, AppQueue, Id);

  // addressed to this instance alone, so opening options here leaves every
  // other viewport on whatever page it was already showing
  const int Instance = Id;
  OptionButton = CreateElement<GenericButton>(
      "instance_menu_option", "options", std::vector<float>{0.5f, 0.56f},
      Device,
      [Instance](EventQueue& Queue, float MouseX, float MouseY) {
        Queue.Enqueue(ChangeStateEvent{AppState::MAIN_OPTION, Instance});
      },
      AppQueue, Id);

}
