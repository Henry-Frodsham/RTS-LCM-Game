//Copyright © 2025 Henry Frodsham
#include "Application.h"
#include "RenderSystem.h"
#include "InputListener.h"
#include "InputTranslator.h"
#include "InputAnalyser.h"

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
	Input.Update();
	
	// temporary measure to display all the connected devices in the overlay
	std::vector<InputTranslator*> DebugTranslators;
	for (const auto& Element : Input.Devices) {
		InputTranslator* HeapAllocTranslator = new InputTranslator(Element.second);
		Input.AddListenerQueue(Element.second, &HeapAllocTranslator->WaitingEvents);
		DebugTranslators.push_back(HeapAllocTranslator);
	}

	InputAnalyser& InputAnalysisSingleton = InputAnalyser::GetInstance();
	while (true) {
		RenderSingleton.RenderFrame(); //render thread func, here for now but delegate in future
		float DT = RenderSingleton.GetDeltaTime();
		Input.Update();

		for (const auto& Translator : DebugTranslators) {
			Translator->Update(DT);
		}

		InputAnalysisSingleton.Update();

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