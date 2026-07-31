// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <vector>

#include "EventQueue.h"
#include "InputDevice.h"

// return event once a ray trace is finished processing, forwarded back using a
// callback
struct EndRayTraceResultEvent {
  bool DidHit;
  Ogre::Vector3f HitPoint;
  Ogre::Vector3f SurfaceNormal;
  uint32_t TileID;
  EndRayTraceResultEvent(bool Hit, Ogre::Vector3f Point, Ogre::Vector3f Normal,
                         uint32_t Tile)
      : DidHit(Hit), HitPoint(Point), SurfaceNormal(Normal), TileID(Tile) {}
};

// start a raytrace, originating in player general control to render system
struct StartRayTraceEvent {
  std::vector<float> Point;
  InputDevice* Device;
  std::function<void(EventQueue*, EndRayTraceResultEvent)> Callback;
  EventQueue* CallQueue;
  StartRayTraceEvent(
      std::vector<float> Pos, InputDevice* Dev,
      std::function<void(EventQueue*, EndRayTraceResultEvent)> Cb,
      EventQueue* Cq)
      : Point(Pos), Device(Dev), Callback(Cb), CallQueue(Cq) {}
};
