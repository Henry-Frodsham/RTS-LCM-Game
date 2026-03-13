// Copyright © 2025 Henry Frodsham
#pragma once

#include "StateEvent.h"
#include "GenericButton.h"
class GameState {
 private:
  EventQueue* AppQueue;
 public:
  GameState(EventQueue* CallBacksQueue);
  ~GameState();

  void Init();
  void OnChangeState(ChangeStateEvent Event);
  
};