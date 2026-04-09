// Copyright © 2025 Henry Frodsham
#include "PausedState.h"
PauseState::PauseState(EventQueue* CallBacksQueue) : AppQueue(CallBacksQueue) {}

void PauseState::Init() {}

void PauseState::OnChangeState(ChangeStateEvent Event) {
  if (Event.NewState == AppState::MENU) {
  } else {
    // hide the play button, or any other buttons
  }
}
