// Copyright (c) 2025 Henry Frodsham
#include "PausedState.h"

PauseState::PauseState(EventQueue* CallBacksQueue)
    : BaseState(CallBacksQueue, AppState::PAUSE) {}
