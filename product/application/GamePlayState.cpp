// Copyright (c) 2025 Henry Frodsham
#include "GamePlayState.h"

GameState::GameState(EventQueue* CallBacksQueue, WorldManager* WorldMng)
    : AppQueue(CallBacksQueue), World(WorldMng) {}
void GameState::Init() {
  World->WorldQueue->Enqueue(ChangeGlobeVisibilityEvent(false));
}
// called same tick as a change state event
void GameState::OnChangeState(ChangeStateEvent Event) {
  if (Event.NewState == AppState::GAME) {
    World->WorldQueue->Enqueue(ChangeGlobeVisibilityEvent(true));
  } else {
    // hide the world, freeze everything, maybe save?
    World->WorldQueue->Enqueue(ChangeGlobeVisibilityEvent(false));
  }
}
