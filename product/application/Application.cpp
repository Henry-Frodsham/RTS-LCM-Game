//Copyright © 2025 Henry Frodsham
#include "Application.h"
#include "RenderSystem.h"
#include "InputListener.h"
#include "InputTranslator.h"

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
	//temporary test
	InputListener Input = InputListener(RenderSingleton.GetSDLWindow());
	InputTranslator Translator = InputTranslator{};
	Input.Update();
	Input.AddListenerQueue(Input.GetDeviceFromSDLId(-1), &Translator.WaitingEvents);

	while (true) {
		RenderSingleton.RenderFrame(); //render thread func, here for now but delegate in future
		Input.Update();
		Translator.WaitingEvents.Dispatch();
		if (Translator.getKeyState('G')) {
			std::cout << "g";
		}
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