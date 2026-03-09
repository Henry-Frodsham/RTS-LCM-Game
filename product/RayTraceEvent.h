#pragma once
#include <OGRE/Ogre.h>

#include <vector>

#include "InputDevice.h"
struct StartRayTraceEvent {
  std::vector<float> Point;
  InputDevice* Device;
  StartRayTraceEvent(std::vector<float> Pos, InputDevice* Dev)
      : Point(Pos), Device(Dev) {}
};
struct EndRayTraceResultEvent {
  Ogre::Ray RayResult;
  EndRayTraceResultEvent(Ogre::Ray Ray) : RayResult(Ray) {}
};
