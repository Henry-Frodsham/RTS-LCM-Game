//Copyright © 2025 Henry Frodsham
#include "Application.h"
#include "RenderSystem.h"

Application::Application() {
	StateManager = ApplicationStateManager();
}

// initialises game into the correct state, by default the game starts at the menu so MENU state
void Application::Start() {
	Init();
	Loop();
}

// sets up rendering engine and input system
bool Application::Init() {
	// get singleton to force init
	RenderSystem &RenderSingleton = RenderSystem::GetInstance();
	RenderSingleton.Init();

	return false;
}

//state reactive loop
void Application::Loop() {
	RenderSystem& RenderSingleton = RenderSystem::GetInstance();
	while (true) {
		RenderSingleton.RenderFrame(); //render thread func, here for now but delegate in future
		if (StateManager.CurrentState == AppState::GAME) {
		}
		else if (StateManager.CurrentState == AppState::MENU){
		}
		else if (StateManager.CurrentState == AppState::PAUSE) {
		}
		else {
			//invalid state
			return;
		}
	}
}