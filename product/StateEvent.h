// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "AppState.h"
struct ChangeStateEvent {
  AppState NewState;
  explicit ChangeStateEvent(AppState State)
      : NewState(State) {}
};
