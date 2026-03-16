// Copyright © 2025 Henry Frodsham
#pragma once
#include "StateEvent.h"
#include "GenericButton.h"

class MenuState {
 private:
  EventQueue* AppQueue;

  GenericButton* PlayButton;
 public:
  MenuState(EventQueue* CallBacksQueue);
  ~MenuState(){}

  void Init();
  void OnChangeState(ChangeStateEvent Event);
};
