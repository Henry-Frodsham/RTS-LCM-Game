// Copyright © 2025 Henry Frodsham
#include "ApplicationStateManager.h"

ApplicationStateManager::ApplicationStateManager()
    : CurrentState(AppState::MENU) {}

void ApplicationStateManager::ChangeApplicationState(ChangeStateEvent Event) {
  CurrentState = Event.NewState;
}
