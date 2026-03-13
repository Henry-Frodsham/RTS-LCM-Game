// Copyright © 2025 Henry Frodsham
#pragma once
#include <iostream>
#include <thread>
#include "EventQueue.h"
#include "EventBus.h"
#include <BS_thread_pool.hpp>
#include "ApplicationStateManager.h"
#include "MenuState.h"
#include "PausedState.h"
#include "GamePlayState.h"

class Application {
 public:
  Application();
  ~Application() {};
  void Start();

  EventQueue* AppQueue;
 private:
  bool Init();

  void Loop();
  BS::thread_pool IndependantThreads;
  ApplicationStateManager StateManager;

  MenuState Menu;
  GameState Game;
  PauseState Pause;

  EventBus* Appbus;
};
