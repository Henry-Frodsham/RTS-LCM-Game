// Copyright © 2025 Henry Frodsham
#pragma once
#include "GenericButton.h"
#include "StateEvent.h"

class MenuState {
 private:
  EventQueue* AppQueue;

 public:
  MenuState(EventQueue* CallBacksQueue);
  ~MenuState() {}

  GenericButton* PlayButton;

  void Init();
  void OnChangeState(ChangeStateEvent Event);
};
