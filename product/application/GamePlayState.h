// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "BaseState.h"
#include "GenericButton.h"
#include "GenericSlider.h"
#include "GlobeEvent.h"
#include "RenderSystem.h"

class GameState : public BaseState {
 public:
  explicit GameState(EventQueue* CallBacksQueue);

 protected:
  void OnInit() override;
  void OnEnter() override;
  void OnExit() override;

 private:
  void SetGlobeVisible(bool Visible);
};
