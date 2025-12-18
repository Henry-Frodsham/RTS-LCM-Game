// Copyright © 2025 Henry Frodsham
#include "Application.h"

#include "InputAnalyser.h"
#include "InputListener.h"
#include "InputTranslator.h"
#include "RenderSystem.h"
#include "WorldManager.h"
#include "instanceOverseer.h"

Application::Application() { StateManager = ApplicationStateManager(); }

// initialises game into the correct state, by default the game starts at the
// menu so MENU state
void Application::Start() {
  Init();
  Loop();
}

// sets up rendering engine and input system
bool Application::Init() {
  // get singleton to force init
  RenderSystem& RenderSingleton = RenderSystem::GetInstance();
  RenderSingleton.Init();

  return false;
}

// state reactive loop
void Application::Loop() {
  RenderSystem& RenderSingleton = RenderSystem::GetInstance();
  // temporary test
  InputListener Input = InputListener(RenderSingleton.GetSDLWindow());
  Input.Update();

  WorldManager WM = WorldManager();
  WM.WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "test", "cube.mesh", "test1", Ogre::Vector3(0.5f, 0.f, -5.f)));
  // temporary measure to display all the connected devices in the overlay

  InstanceOverseer Instances = InstanceOverseer(&Input);

  InputAnalyser& InputAnalysisSingleton = InputAnalyser::GetInstance();
  while (true) {
    RenderSingleton.RenderFrame();  // all interactions with ogre need to be run in the main thread

    std::thread WorldManagerThread = std::thread(&WorldManager::update, &WM);

    float DT = RenderSingleton.GetDeltaTime();
    std::thread InputThread = std::thread(&InputListener::Update, &Input);

    Instances.ReviseAndUpdate(DT);

    std::thread InputAnalysisThread = std::thread(&InputAnalyser::Update, &InputAnalysisSingleton);

    if (StateManager.CurrentState == AppState::GAME) {
    } else if (StateManager.CurrentState == AppState::MENU) {
    } else if (StateManager.CurrentState == AppState::PAUSE) {
    } else {
      // invalid state
      return;
    }
    WorldManagerThread.join();
    InputThread.join();
    InputAnalysisThread.join();
  }
}
