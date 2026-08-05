// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <entt/entt.hpp>
#include <vector>

#include "Biome.h"
#include "EventQueue.h"
#include "Globe.h"
#include "InputDevice.h"

struct RayPickResult {
  GlobeRayHit Terrain{};  // DidHit, HitPoint, Normal, TileID, Biome
  entt::entity HitEntity{entt::null};
  float EntityDistance{0.f};

  bool HasEntity() const { return HitEntity != entt::null; }
};

// start a raytrace, originating in player general control to render system
struct StartRayTraceEvent {
  std::vector<float> Point;
  InputDevice* Device;
  std::function<void(EventQueue*, RayPickResult)> Callback;
  EventQueue* CallQueue;
  StartRayTraceEvent(std::vector<float> Pos, InputDevice* Dev,
                     std::function<void(EventQueue*, RayPickResult)> Cb,
                     EventQueue* Cq)
      : Point(Pos), Device(Dev), Callback(Cb), CallQueue(Cq) {}
};
