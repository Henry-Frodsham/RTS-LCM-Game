//Copyright © 2025 Henry Frodsham
#include "Application.h"
#include "RenderSystem.h"
// initialises game into the correct state, by default the game starts at the menu so MENU state
void Application::Start() {
	Init();
}

// sets up rendering engine and input system
bool Application::Init() {
	// get singleton to force init
	RenderSystem &RenderSingleton = RenderSystem::GetInstance();


	return false;
}