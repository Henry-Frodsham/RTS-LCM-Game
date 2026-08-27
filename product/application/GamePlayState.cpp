// Copyright (c) 2025 Henry Frodsham
#include "GamePlayState.h"

#include <vector>

GameState::GameState(EventQueue* CallBacksQueue)
    : BaseState(CallBacksQueue, AppState::GAME) {}

void GameState::OnInit() {
  SetGlobeVisible(false);
}

void GameState::OnEnter() { SetGlobeVisible(true); }

void GameState::OnExit() { SetGlobeVisible(false); }

void GameState::SetGlobeVisible(bool Visible) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(ChangeGlobeVisibilityEvent(Visible));
}
