// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <string>
#include <vector>

#include "InputDevice.h"
#include "RenderSystem.h"

class GenericButton {
 public:
  GenericButton(std::string ButtonName, std::string ButtonText,
                std::vector<float> Position, InputDevice* DeviceToRespondTo,
                std::function<void(EventQueue&)> PressCallback,
                EventQueue* QueueForCallBack, int ThreadId);
  void ChangeVisibility(bool Visible);
  void MaintainScaling();
 private:
  std::string Name;
  std::string Text;

  std::vector<float> Pos;

  InputDevice* Device;

  std::function<void(EventQueue&)> CallBackOnPress;

  EventQueue* CallBackQueue;
  int Id;
};
