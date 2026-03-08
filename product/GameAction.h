// Copyright © 2025 Henry Frodsham
#pragma once
#include "InputDevice.h"
// unifies game input from mouse and keyboard alike, makes managing input in
// actual game logic more manageable
enum GameAction {
  USE,
  CONTXT,
  BACK,
};

struct ActionContext {
  float MouseX, MouseY;
  bool JustPressed;
  InputDevice* ActioningDevice;
  int ThreadId;
  ActionContext(float X, float Y, bool Pressed, int ThreadNum, InputDevice* Dev)
      : MouseX(X),
        MouseY(Y),
        JustPressed(Pressed),
        ThreadId(ThreadNum), ActioningDevice(Dev) {}
};