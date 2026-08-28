// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <entt/entt.hpp>
#include <utility>  // NOLINT(build/include_order)
#include <vector>   // NOLINT(build/include_order)

#include "Biome.h"

struct NotifyRayResult {
  Ogre::Vector3f Pos;
  Ogre::Vector3f SurfaceNormal;
  BiomeType Biome;

  // the add-to-selection modifier was held for the gesture that produced
  // this. a click on empty ground normally clears the selection, and holding
  // the modifier is how a player says they didnt mean to
  bool Additive;

  NotifyRayResult(Ogre::Vector3f Position, Ogre::Vector3f SN, BiomeType BT,
                  bool Add = false)
      : Pos(Position), SurfaceNormal(SN), Biome(BT), Additive(Add) {}
};

struct NotifyEntityResult {
  entt::entity Entity;

  // same modifier as NotifyRayResult carries. on an entity it means "add this
  // one to what is already selected", and on one thats already selected it
  // means "take it back out again"
  bool Additive;

  NotifyEntityResult(entt::entity Ent, bool Add = false)
      : Entity(Ent), Additive(Add) {}
};

// everything a finished rubber band drag enclosed. ownership hasnt been
// checked yet - these are simply the entities that were inside the box, and
// each still has to go through the same TrySelectEntityEvent validation a
// clicked one does
struct NotifyBoxSelectResult {
  std::vector<entt::entity> Entities;
  bool Additive;

  NotifyBoxSelectResult(std::vector<entt::entity> Ents, bool Add)
      : Entities(std::move(Ents)), Additive(Add) {}
};

struct SelectEntitySuccessEvent {
  entt::entity Entity;
  SelectEntitySuccessEvent(entt::entity Ent) : Entity(Ent) {}
};
