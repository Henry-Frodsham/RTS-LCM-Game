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

  // copied straight off the request. a pick crosses two queues before anyone
  // acts on it, so the gesture's modifier travels with the answer rather than
  // being remembered on the side and read back out of date
  bool Additive{false};

  bool HasEntity() const { return HitEntity != entt::null; }
};

// everything a box select found, in no particular order. the same "ask the
// render side to pick, answer on the queue you were handed" shape as
// RayPickResult, because a rubber band is a pick with an area instead of a
// point
struct BoxPickResult {
  std::vector<entt::entity> Entities;

  // carried straight through from the gesture that asked. the pick itself has
  // no opinion on what should happen to the current selection, it only has to
  // make sure the answer still knows which gesture it belongs to by the time
  // it crosses back over the queue
  bool Additive{false};

  bool IsEmpty() const { return Entities.empty(); }
};

// start a raytrace, originating in player general control to render system
struct StartRayTraceEvent {
  std::vector<float> Point;
  InputDevice* Device;
  std::function<void(EventQueue*, RayPickResult)> Callback;
  EventQueue* CallQueue;

  // the add-to-selection modifier the gesture was made with. meaningless for
  // the hold-to-preview traces, which is why it defaults off
  bool Additive;

  StartRayTraceEvent(std::vector<float> Pos, InputDevice* Dev,
                     std::function<void(EventQueue*, RayPickResult)> Cb,
                     EventQueue* Cq, bool Add = false)
      : Point(Pos), Device(Dev), Callback(Cb), CallQueue(Cq), Additive(Add) {}
};

// start a box select, originating in player general control to render system.
// the rectangle is in the same normalised 0-1 viewport space a
// StartRayTraceEvent point is, and may be given corners in any order
struct StartBoxSelectEvent {
  std::vector<float> Origin;
  std::vector<float> Corner;
  InputDevice* Device;
  bool Additive;
  std::function<void(EventQueue*, BoxPickResult)> Callback;
  EventQueue* CallQueue;

  StartBoxSelectEvent(std::vector<float> Start, std::vector<float> End,
                      InputDevice* Dev, bool Add,
                      std::function<void(EventQueue*, BoxPickResult)> Cb,
                      EventQueue* Cq)
      : Origin(Start),
        Corner(End),
        Device(Dev),
        Additive(Add),
        Callback(Cb),
        CallQueue(Cq) {}
};
