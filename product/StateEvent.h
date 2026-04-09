// Copyright © 2025 Henry Frodsham
#pragma once
#include "AppState.h"
struct ChangeStateEvent {
  AppState NewState;
  ChangeStateEvent(AppState State) : NewState(State) {}
};
