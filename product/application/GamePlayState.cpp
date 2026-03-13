// Copyright © 2025 Henry Frodsham
#include "GamePlayState.h"

GameState::GameState(EventQueue* CallBacksQueue) : AppQueue(CallBacksQueue) {}
void GameState::Init() {}
void GameState::OnChangeState(ChangeStateEvent Event) {
  if (Event.NewState == AppState::GAME) {
  } else {
	//hide the world, freeze everything, maybe save?
  }
}