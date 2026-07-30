// Copyright (c) 2025 Henry Frodsham
#include "GamePlayState.h"

GameState::GameState(EventQueue* CallBacksQueue) : AppQueue(CallBacksQueue) {}
void GameState::Init() {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(ChangeGlobeVisibilityEvent(false));
}
// called same tick as a change state event
void GameState::OnChangeState(ChangeStateEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  if (Event.NewState == AppState::GAME) {
    Rs.RenderQueue->Enqueue(ChangeGlobeVisibilityEvent(true));
  } else {
    // hide the world, freeze everything, maybe save?
    Rs.RenderQueue->Enqueue(ChangeGlobeVisibilityEvent(false));
  }
}
