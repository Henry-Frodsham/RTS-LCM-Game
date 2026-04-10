// Copyright (c) 2025 Henry Frodsham
#pragma once

#include "GenericButton.h"
#include "StateEvent.h"
#include "WorldManager.h"
class GameState {
 private:
  EventQueue* AppQueue;
  WorldManager* World;

 public:
  GameState(EventQueue* CallBacksQueue, WorldManager* WorldMng);
  ~GameState() {}

  void Init();
  void OnChangeState(ChangeStateEvent Event);
};
