// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <vector>

#include "EventQueue.h"
#include "InputDevice.h"

// return event once a ray trace is finished processing, forwarded back using a
// callback
struct EndRayTraceResultEvent {
  Ogre::RaySceneQueryResult& RayResult;
  Ogre::Ray Ray;
  EndRayTraceResultEvent(Ogre::RaySceneQueryResult& Ray, Ogre::Ray R)
      : RayResult(Ray), Ray(R) {}
};

// start a raytrace, originating in player general control to render system
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
