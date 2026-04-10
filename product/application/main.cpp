// Copyright (c) 2025 Henry Frodsham
#include <doctest/doctest.h>

#include "Application.h"

// stop SDL hijacking main
#undef main

// startup function, delegates responsibility to the application class
int main(int argc, char** argv) {
  Application* MainApp = new Application();

  MainApp->Start();

  // shutdown
  delete MainApp;
  return 0;
}
