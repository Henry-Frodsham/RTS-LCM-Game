#pragma once
#include "InputDevice.h"

struct CursorMovementEvent {
  std::vector<float> AbsoluteXY;
  std::vector<float> RelativeXY;
  int ThreadNumber;
  InputDevice* Device;
  CursorMovementEvent(std::vector<float> AbXY, std::vector<float> RelXY,
                      int ThreadNum, InputDevice* Dev)
      : AbsoluteXY(AbXY),
        RelativeXY(RelXY),
        ThreadNumber(ThreadNum),
        Device(Dev) {}
};
