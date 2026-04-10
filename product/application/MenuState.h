// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "GenericButton.h"
#include "StateEvent.h"

class MenuState {
 private:
  EventQueue* AppQueue;

 public:
  explicit MenuState(EventQueue* CallBacksQueue);
  ~MenuState() {}

  GenericButton* PlayButton;

  void Init();
  void OnChangeState(ChangeStateEvent Event);
};
