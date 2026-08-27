// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "BaseState.h"
#include "GenericButton.h"

class MenuState : public BaseState {
 public:
  explicit MenuState(EventQueue* CallBacksQueue);

 protected:
  void OnInit() override;

 private:
  GenericButton* PlayButton;
  GenericButton* OptionButton;
  GenericButton* WorldGenerationButton;
};
