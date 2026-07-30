// Copyright (c) 2025 Henry Frodsham
#include "Application.h"

#include <vector>
Application::Application()
    : IndependantThreads(std::thread::hardware_concurrency()),
      RenderSingleton(RenderSystem::GetInstance()),
      Input(InputListener(InitAndGetWindow(RenderSingleton))),
      WM(WorldManager()),
      Instances(InstanceOverseer(&UpdateAndReturn(), WM.CompFactory)),
      StateManager(ApplicationStateManager()),
      Appbus(new EventBus()),
      AppQueue(new EventQueue(Appbus)),
      Menu(MenuState(AppQueue)),
      Game(GameState(AppQueue)),
      Pause(PauseState(AppQueue)) {}

// initialises game into the correct state, by default the game starts at the
// menu so MENU state
void Application::Start() {
  Init();
  Loop();
}

// initialise the app and its states
bool Application::Init() {
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

// helper method to enforce correct initialising order of worldmanager
SDL_Window* Application::InitAndGetWindow(RenderSystem& Render) {
  Render.Init();
  return Render.GetSDLWindow();
}

InputListener& Application::UpdateAndReturn() {
  Input.Update();
  return Input;
}
// state reactive loop
void Application::Loop() {
  RenderSystem& RenderSingleton = RenderSystem::GetInstance();
  InputAnalyser& InputAnalysisSingleton = InputAnalyser::GetInstance();
  while (true) {
    AppQueue->Dispatch();
    std::vector<std::future<void>> Futures;
    Futures.reserve(3);

    RenderSingleton.RenderFrame();  // all interactions with ogre need to be run
                                    // in the main thread
    float DT = RenderSingleton.GetDeltaTime();
    Futures.push_back(
        IndependantThreads.submit_task([this, DT]() { WM.update(DT); }));

    Futures.push_back(
        IndependantThreads.submit_task([this]() { Input.Update(); }));

    Instances.ReviseAndUpdate(DT);

    Futures.push_back(
        IndependantThreads.submit_task([&InputAnalysisSingleton, DT]() {
          InputAnalysisSingleton.Update(DT);
        }));

    if (StateManager.CurrentState == AppState::GAME) {
    } else if (StateManager.CurrentState == AppState::MENU) {
      Menu.PlayButton->MaintainScaling();
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
