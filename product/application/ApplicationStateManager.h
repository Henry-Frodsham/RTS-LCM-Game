// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "AppState.h"
#include "GamePlayState.h"
#include "MenuState.h"
#include "PausedState.h"
#include "StateEvent.h"

// controls the state of the application by controlling which update function to
// call
class ApplicationStateManager {
 public:
  ApplicationStateManager();
  ~ApplicationStateManager() {}

  AppState CurrentState;

  void ChangeApplicationState(ChangeStateEvent Event);

 private:
};
