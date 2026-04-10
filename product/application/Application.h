// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <BS_thread_pool.hpp>
#include <iostream>  // NOLINT(build/include_order)
#include <thread>  // NOLINT(build/include_order)

#include "ApplicationStateManager.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "GamePlayState.h"
#include "InputAnalyser.h"
#include "InputListener.h"
#include "InputTranslator.h"
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
  GameState Game;
  PauseState Pause;
};
