// Copyright © 2025 Henry Frodsham
#pragma once
#include <iostream>
#include <thread>

#include "ApplicationStateManager.h"

class Application {
 public:
  Application();
  ~Application() {};
  void Start();

 private:
  bool Init();

  void Loop();

  ApplicationStateManager StateManager;
};
