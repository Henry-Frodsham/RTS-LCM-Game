#pragma once
#include "GamePlayState.h"
#include "MenuState.h"
#include "PausedState.h"

enum AppState {
	GAME,
	MENU,
	PAUSE
};

class ApplicationStateManager {
public:
	ApplicationStateManager() {};
	~ApplicationStateManager() {};

private:
};
