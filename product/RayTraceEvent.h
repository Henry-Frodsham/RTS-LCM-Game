#pragma once
#include <OGRE/Ogre.h>

#include <vector>

#include "InputDevice.h"
struct StartRayTraceEvent {
  std::vector<float> Point;
  Ogre::RaySceneQuery* RaySceneQuery = nullptr;
  InputDevice* Device;
  StartRayTraceEvent(std::vector<float> Pos, InputDevice* Dev)
      : Point(Pos), Device(Dev) {}
};
struct EndRayTraceResultEvent {
  Ogre::RaySceneQueryResult& RayResult;
  EndRayTraceResultEvent(Ogre::RaySceneQueryResult& Ray) : RayResult(Ray) {}
};
