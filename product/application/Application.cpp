// Copyright © 2025 Henry Frodsham
#include "Application.h"

#include "InputAnalyser.h"
#include "InputListener.h"
#include "InputTranslator.h"
#include "RenderSystem.h"
#include "StateEvent.h"
#include "WorldManager.h"
#include "instanceOverseer.h"

Application::Application()
    : IndependantThreads(std::thread::hardware_concurrency()) {
  StateManager = ApplicationStateManager();
  Appbus = new EventBus();
  AppQueue = new EventQueue(Appbus);

  Menu = MenuState(AppQueue);
  Game = GameState(AppQueue);
  Pause = PauseState(AppQueue);
}

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

  Menu.Init();
  Game.Init();
  Pause.Init();

  Appbus->Subscribe<ChangeStateEvent>(
      std::bind(&ApplicationStateManager::ChangeApplicationState, &StateManager,
                std::placeholders::_1));
  Appbus->Subscribe<ChangeStateEvent>(
      std::bind(&MenuState::OnChangeState, &Menu, std::placeholders::_1));
  Appbus->Subscribe<ChangeStateEvent>(
      std::bind(&GameState::OnChangeState, &Game, std::placeholders::_1));
  Appbus->Subscribe<ChangeStateEvent>(
      std::bind(&PauseState::OnChangeState, &Pause, std::placeholders::_1));

  return false;
}

// state reactive loop
void Application::Loop() {
  RenderSystem& RenderSingleton = RenderSystem::GetInstance();

  InputListener Input = InputListener(RenderSingleton.GetSDLWindow());
  Input.Update();

  WorldManager WM = WorldManager();

  InstanceOverseer Instances = InstanceOverseer(&Input);

  InputAnalyser& InputAnalysisSingleton = InputAnalyser::GetInstance();
  while (true) {
    AppQueue->Dispatch();
    std::vector<std::future<void>> Futures;
    Futures.reserve(3);

    RenderSingleton.RenderFrame();  // all interactions with ogre need to be run
                                    // in the main thread

    Futures.push_back(IndependantThreads.submit_task([&WM]() { WM.update(); }));
    float DT = RenderSingleton.GetDeltaTime();

    Futures.push_back(
        IndependantThreads.submit_task([&Input]() { Input.Update(); }));

    Instances.ReviseAndUpdate(DT);

    Futures.push_back(
        IndependantThreads.submit_task([&InputAnalysisSingleton, DT]() {
          InputAnalysisSingleton.Update(DT);
        }));

    if (StateManager.CurrentState == AppState::GAME) {
    } else if (StateManager.CurrentState == AppState::MENU) {
    } else if (StateManager.CurrentState == AppState::PAUSE) {
    } else {
      // invalid state
      return;
    }
    for (auto& Future : Futures) {
      Future.wait();
    }
  }
}
