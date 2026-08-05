// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include "Biome.h"


struct NotifyRayResult {
  Ogre::Vector3f Pos;
  Ogre::Vector3f SurfaceNormal;
  BiomeType Biome;
  NotifyRayResult(Ogre::Vector3f Position, Ogre::Vector3f SN, BiomeType BT)
      : Pos(Position), SurfaceNormal(SN), Biome(BT) {}

};

struct NotifyEntityResult {
  entt::entity Entity;
  NotifyEntityResult (entt::entity Ent) : Entity(Ent){
  }
};


struct SelectEntitySuccessEvent {
  entt::entity Entity;
  SelectEntitySuccessEvent(entt::entity Ent) : Entity(Ent) {}
};