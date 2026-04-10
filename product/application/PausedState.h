// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "GenericButton.h"
#include "StateEvent.h"

class PauseState {
 private:
  EventQueue* AppQueue;

 public:
  explicit PauseState(EventQueue* CallBacksQueue);
  ~PauseState() {}

  void Init();
  void OnChangeState(ChangeStateEvent Event);
};
