// Copyright © 2025 Henry Frodsham
#pragma once

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
  ActionContext(float X, float Y, bool Pressed)
      : MouseX(X), MouseY(Y), JustPressed(Pressed) {}
};