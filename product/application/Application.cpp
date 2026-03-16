// Copyright © 2025 Henry Frodsham
#include "Application.h"



Application::Application()
    : IndependantThreads(std::thread::hardware_concurrency()),RenderSingleton(RenderSystem::GetInstance()), Input(InputListener(InitAndGetWindow(RenderSingleton))), WM(WorldManager()), Instances(InstanceOverseer(&UpdateAndReturn())), StateManager(ApplicationStateManager()), Appbus(new EventBus()), AppQueue(new EventQueue(Appbus)), Menu(MenuState(AppQueue)), Game(GameState(AppQueue)), Pause(PauseState(AppQueue)) {
}

// initialises game into the correct state, by default the game starts at the
// menu so MENU state
void Application::Start() {
  Init();
  Loop();
}

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

SDL_Window* Application::InitAndGetWindow(RenderSystem& Render){
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
    Futures.push_back(IndependantThreads.submit_task([this]() { WM.update(); }));
    float DT = RenderSingleton.GetDeltaTime();

    Futures.push_back(
        IndependantThreads.submit_task([this]() { Input.Update(); }));

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
