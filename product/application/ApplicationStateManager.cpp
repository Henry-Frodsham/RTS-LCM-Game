// Copyright (c) 2025 Henry Frodsham
#include "ApplicationStateManager.h"

ApplicationStateManager::ApplicationStateManager()
    : BroadcastState(AppState::MENU) {}

void ApplicationStateManager::RegisterState(BaseState* State) {
  if (State == nullptr) {
    return;
  }
  States.push_back(State);
}

void ApplicationStateManager::ChangeApplicationState(ChangeStateEvent Event) {
  if (Event.Owner == StateOwnerAll) {
    // a broadcast overrules wherever each instance had wandered off to, so the
    // per owner states are dropped rather than left to contradict it
    BroadcastState = Event.NewState;
    OwnerStates.clear();
    return;
  }

  OwnerStates[Event.Owner] = Event.NewState;
}

AppState ApplicationStateManager::GetStateFor(int Owner) const {
  auto Found = OwnerStates.find(Owner);

  if (Found == OwnerStates.end()) {
    return BroadcastState;
  }
  return Found->second;
}

void ApplicationStateManager::MaintainActiveStateUI() {
  for (BaseState* State : States) {
    if (State->IsActive()) {
      State->MaintainScaling();
    }
  }
}
