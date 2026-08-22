// Copyright (c) 2025 Henry Frodsham
#pragma once

#include "GenericButton.h"
#include "GenericSlider.h"
#include "StateEvent.h"
#include "GlobeEvent.h"
#include "RenderSystem.h"

class GameState {
 private:
  EventQueue* AppQueue;

 public:
  GameState(EventQueue* CallBacksQueue);
  ~GameState() {}

  // dev-only control, not tied to any gameplay value yet - exists so a value
  // can be tweaked live at runtime while debugging
  GenericSlider* DebugSlider;

  void Init();
  void OnChangeState(ChangeStateEvent Event);
};
