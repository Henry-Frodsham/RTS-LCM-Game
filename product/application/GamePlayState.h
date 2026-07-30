// Copyright (c) 2025 Henry Frodsham
#pragma once

#include "GenericButton.h"
#include "StateEvent.h"
#include "GlobeEvent.h"
#include "RenderSystem.h"

class GameState {
 private:
  EventQueue* AppQueue;

 public:
  GameState(EventQueue* CallBacksQueue);
  ~GameState() {}

  void Init();
  void OnChangeState(ChangeStateEvent Event);
};
