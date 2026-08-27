// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "BaseState.h"
#include "GenericButton.h"

class PauseState : public BaseState {
 public:
  explicit PauseState(EventQueue* CallBacksQueue);
};
