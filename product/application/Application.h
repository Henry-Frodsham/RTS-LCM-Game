// Copyright © 2025 Henry Frodsham
#pragma once
#include <iostream>
#include <thread>
#include <BS_thread_pool.hpp>
#include "ApplicationStateManager.h"

class Application {
 public:
  Application();
  ~Application() {};
  void Start();

 private:
  bool Init();

  void Loop();
  BS::thread_pool IndependantThreads;
  ApplicationStateManager StateManager;
};
