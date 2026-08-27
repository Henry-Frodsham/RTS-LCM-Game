// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <BS_thread_pool.hpp>
#include <iostream>  // NOLINT(build/include_order)
#include <memory>  // NOLINT(build/include_order)
#include <thread>  // NOLINT(build/include_order)
#include <unordered_set>  // NOLINT(build/include_order)
#include <vector>  // NOLINT(build/include_order)

#include "ApplicationStateManager.h"
#include "BaseState.h"
#include "ConfigEvent.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "GamePlayState.h"
#include "GlobeOptionState.h"
#include "InputAnalyser.h"
#include "InputListener.h"
#include "InputTranslator.h"
#include "InstanceMenuState.h"
#include "InstanceOptionState.h"
#include "LoadingState.h"
#include "MainOptionState.h"
#include "MenuState.h"
#include "PausedState.h"
#include "RenderSystem.h"
#include "StateEvent.h"
#include "WorldManager.h"
#include "instanceOverseer.h"
class Application {
 public:
  Application();
  ~Application() {}
  void Start();

 private:
  EventBus* Appbus;
  EventQueue* AppQueue;
  bool Init();

  void Loop();
  SDL_Window* InitAndGetWindow(RenderSystem& Render);
  InputListener& UpdateAndReturn();
  BS::thread_pool IndependantThreads;
  ApplicationStateManager StateManager;
  RenderSystem& RenderSingleton;

  InputListener Input;
  WorldManager WM;
  InstanceOverseer Instances;
  MenuState Menu;
  LoadingState Loading;
  GameState Game;
  PauseState Pause;
  MainOptionState Options;
  GlobeOptionState GlobeOptions;

  // every state built at startup, as the base type - the only list Init needs
  std::vector<BaseState*> AllStates();

  // start driving a state: it is scaled with the rest, follows every later
  // transition, builds its UI, and is brought up to whichever state the app is
  // already in, so a state added mid run is no different to one added at boot
  void RegisterState(BaseState* State);

  // hand a config reload to the render system and to every instance, each on
  // the thread it is safe to touch it from
  void BroadcastConfigChange(ConfigAppliedEvent Event);

  // a controller instance only exists once its pad has been plugged in, so its
  // pages are built the first frame after that rather than up front
  void SyncInstanceStates();

  std::vector<std::unique_ptr<InstanceMenuState>> InstanceMenus;
  std::vector<std::unique_ptr<InstanceOptionState>> InstanceOptions;

  // instances already dealt with, so an instance is never handed a second set
  // of pages, and one deliberately skipped is not reconsidered every frame
  std::unordered_set<int> InstancesWithPages;
};
