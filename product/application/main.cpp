// Copyright © 2025 Henry Frodsham
#include <doctest/doctest.h>

#include "Application.h"

// startup function, delegates responsibility to the application class
int main(int argc, char** argv) {
  Application* MainApp = new Application();

  MainApp->Start();

  // shutdown
  delete MainApp;
  return 0;
}
