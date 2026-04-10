// Copyright (c) 2025 Henry Frodsham
#include "PausedState.h"
PauseState::PauseState(EventQueue* CallBacksQueue) : AppQueue(CallBacksQueue) {}

void PauseState::Init() {}

// called any time the app changes state
void PauseState::OnChangeState(ChangeStateEvent Event) {
  if (Event.NewState == AppState::MENU) {
  } else {
    // hide the play button, or any other buttons
  }
}
