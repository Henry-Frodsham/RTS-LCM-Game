// Copyright (c) 2025 Henry Frodsham
#include "Application.h"

#include <memory>
#include <utility>
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
      Menu(AppQueue),
      Loading(AppQueue),
      Game(AppQueue),
      Pause(AppQueue),
      Options(AppQueue),
      GlobeOptions(AppQueue) {}

std::vector<BaseState*> Application::AllStates() {
  return {&Menu, &Loading, &Game, &Pause, &Options, &GlobeOptions};
}

// initialises game into the correct state, by default the game starts at the
// menu so MENU state
void Application::Start() {
  Init();
  Loop();
}

// initialise the app and its states
bool Application::Init() {
  Appbus->Subscribe<ChangeStateEvent>(
      std::bind(&ApplicationStateManager::ChangeApplicationState, &StateManager,
                std::placeholders::_1));
  Appbus->Subscribe<ConfigAppliedEvent>(std::bind(
      &Application::BroadcastConfigChange, this, std::placeholders::_1));

  for (BaseState* State : AllStates()) {
    RegisterState(State);
  }

  return false;
}

// states start with their UI hidden, so a state is entered here through the
// same path as every later transition rather than being trusted to show itself
void Application::RegisterState(BaseState* State) {
  StateManager.RegisterState(State);
  Appbus->Subscribe<ChangeStateEvent>(
      std::bind(&BaseState::OnChangeState, State, std::placeholders::_1));
  State->Init();
  State->OnChangeState(
      ChangeStateEvent{StateManager.GetStateFor(State->GetOwner()),
                       State->GetOwner()});
}

// an options page has written a config file, and the things that read it are
// spread across the render thread and every instance thread. neither is safe
// to call into from here, so both are handed the news through the queue they
// already drain on their own thread - the render queue inside RenderFrame and
// the instance queue inside ReviseAndUpdate
//
// every subscriber is told about every file. a reader knows which config is
// its own far better than this does, and filtering here would mean this
// function had to be edited every time something new started reading a config
void Application::BroadcastConfigChange(ConfigAppliedEvent Event) {
  RenderSystem::GetInstance().RenderQueue->Enqueue(Event);
  Instances.InstanceQueue->Enqueue(Event);
}

// give every controller instance its own menu and options pages. the keyboard
// and mouse instance is left on the main menu, since it is the one that starts
// the game, so it is the only one whose menu offers play instead of ready
void Application::SyncInstanceStates() {
  for (GameInstance* Instance : Instances.GetInstances()) {
    if (InstancesWithPages.count(Instance->InstanceNumber) > 0) {
      continue;
    }
    InstancesWithPages.insert(Instance->InstanceNumber);

    if (Instance->InstanceNumber == 1 || Instance->InstanceDevice == nullptr ||
        Instance->InstanceDevice->InputType != InputDeviceType::CONTROLLER) {
      continue;
    }

    InstanceMenus.push_back(std::make_unique<InstanceMenuState>(
        AppQueue, Instance->InstanceDevice, Instance->InstanceNumber));
    RegisterState(InstanceMenus.back().get());

    InstanceOptions.push_back(std::make_unique<InstanceOptionState>(
        AppQueue, Instance->InstanceDevice, Instance->InstanceNumber));
    RegisterState(InstanceOptions.back().get());
  }
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

    // an instance registered by the update above has no pages yet, and both it
    // and the render system it builds them through are only safe to touch from
    // this thread
    SyncInstanceStates();

    Futures.push_back(
        IndependantThreads.submit_task([&InputAnalysisSingleton, DT]() {
          InputAnalysisSingleton.Update(DT);
        }));

    StateManager.MaintainActiveStateUI();

    for (auto& Future : Futures) {
      Future.wait();
    }
  }
}
