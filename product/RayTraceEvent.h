#pragma once
#include <OGRE/Ogre.h>

#include <vector>

#include "EventQueue.h"
#include "InputDevice.h"

struct EndRayTraceResultEvent {
  Ogre::RaySceneQueryResult& RayResult;
  EndRayTraceResultEvent(Ogre::RaySceneQueryResult& Ray) : RayResult(Ray) {}
};
struct StartRayTraceEvent {
  std::vector<float> Point;
  Ogre::RaySceneQuery* RaySceneQuery = nullptr;
  InputDevice* Device;

  std::function<void(EventQueue*, EndRayTraceResultEvent)> Callback;
  EventQueue* CallQueue;

  StartRayTraceEvent(std::vector<float> Pos, InputDevice* Dev,
      std::function<void(EventQueue*, EndRayTraceResultEvent)> Cb,
      EventQueue* Cq)
      : Point(Pos), Device(Dev), Callback(Cb), CallQueue(Cq) {}
};

