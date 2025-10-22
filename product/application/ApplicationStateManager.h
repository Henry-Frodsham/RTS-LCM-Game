//Copyright © 2025 Henry Frodsham
#pragma once
#include "GamePlayState.h"
#include "MenuState.h"
#include "PausedState.h"

// the different states the game can be in, used to prevent too much logic being processed when unnecessary
enum AppState {
	GAME,
	MENU,
	PAUSE
};

// controls the state of the application by controlling which update function to call
class ApplicationStateManager {
public:
	ApplicationStateManager();
	~ApplicationStateManager() {};

	AppState CurrentState;
private:
	
};
