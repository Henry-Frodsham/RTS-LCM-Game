#pragma once
#include <OGRE/Ogre.h>

#include <vector>

#include "EventQueue.h"
#include "InputDevice.h"

struct EndRayTraceResultEvent {
  Ogre::RaySceneQueryResult& RayResult;
  Ogre::Ray Ray;
  EndRayTraceResultEvent(Ogre::RaySceneQueryResult& Ray, Ogre::Ray R)
      : RayResult(Ray), Ray(R) {}
};
struct StartRayTraceEvent {
  std::vector<float> Point;
  Ogre::RaySceneQuery* RaySceneQuery = nullptr;
  InputDevice* Device;

  std::function<void(EventQueue*, EndRayTraceResultEvent)> Callback;
  EventQueue* CallQueue;

  StartRayTraceEvent(
      std::vector<float> Pos, InputDevice* Dev,
      std::function<void(EventQueue*, EndRayTraceResultEvent)> Cb,
      EventQueue* Cq)
      : Point(Pos), Device(Dev), Callback(Cb), CallQueue(Cq) {}
};
