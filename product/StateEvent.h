// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "AppState.h"

// a request to move one instance, or the whole application, into a new state.
// Owner defaults to every instance because the transitions that are nobody's in
// particular - into and out of the game itself - are the ones that broadcast
struct ChangeStateEvent {
  AppState NewState;
  int Owner;

  explicit ChangeStateEvent(AppState State, int OwnedBy = StateOwnerAll)
      : NewState(State), Owner(OwnedBy) {}
};
