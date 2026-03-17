// Copyright © 2025 Henry Frodsham
#pragma once

#include "StateEvent.h"
#include "GenericButton.h"
#include "WorldManager.h"
class GameState {
 private:
  EventQueue* AppQueue;
  WorldManager* World;
 public:
  GameState(EventQueue* CallBacksQueue, WorldManager* WorldMng);
  ~GameState(){}

  void Init();
  void OnChangeState(ChangeStateEvent Event);
  
};