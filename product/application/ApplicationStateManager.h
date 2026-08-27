// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <unordered_map>
#include <vector>

#include "AppState.h"
#include "BaseState.h"
#include "StateEvent.h"

// controls the state of the application by controlling which update function to
// call. in split screen there is no single current state - each instance sits
// in its own - so a state is tracked per owner, over the top of the last state
// broadcast to everyone
class ApplicationStateManager {
 public:
  ApplicationStateManager();
  ~ApplicationStateManager() {}

  void RegisterState(BaseState* State);

  void ChangeApplicationState(ChangeStateEvent Event);

  // where an instance currently is. an instance nobody has moved individually
  // is wherever the application as a whole last went, which is also what an
  // instance that has only just been plugged in inherits
  AppState GetStateFor(int Owner) const;

  // rescales the UI of whichever states are active, so no caller ever has to
  // name an individual element
  void MaintainActiveStateUI();

 private:
  std::vector<BaseState*> States;

  AppState BroadcastState;
  std::unordered_map<int, AppState> OwnerStates;
};
