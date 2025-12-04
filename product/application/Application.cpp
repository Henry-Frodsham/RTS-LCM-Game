//Copyright © 2025 Henry Frodsham
#include "Application.h"
#include "RenderSystem.h"
#include "InputListener.h"
#include "InputTranslator.h"
#include "InputAnalyser.h"
#include "WorldManager.h"
#include "instanceOverseer.h"

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

	WorldManager WM = WorldManager();
	WM.WorldQueue->Enqueue(CreateMeshWorldEntityEvent("test","cube.mesh","test1", Ogre::Vector3(0.5f,0.f,-5.f)));
	// temporary measure to display all the connected devices in the overlay

	InstanceOverseer Instances = InstanceOverseer(&Input);

	InputAnalyser& InputAnalysisSingleton = InputAnalyser::GetInstance();
	while (true) {
		RenderSingleton.RenderFrame(); //render thread func, here for now but delegate in future
		WM.update();
		float DT = RenderSingleton.GetDeltaTime();
		Input.Update();

		Instances.ReviseAndUpdate(DT);

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