// Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <entt/entt.hpp>
#include <string>

#include "Player.h"

// these events are for convenience only and dont provide an architectural
// benefit they are simply blueprints to create simple entities quickly

// full initial entity creation, without an attached mesh
// an entity of this type has no observable behaviour by itself
// creates an ogre::scenenode
struct CreateBasicWorldEntityEvent {
  // the name of the ogre::sceneNode this entity will be attached to / create
  std::string NodeName;
  Ogre::Vector3 InitialPosition;
  CreateBasicWorldEntityEvent(std::string NodeN, Ogre::Vector3 InitP)
      : NodeName(NodeN), InitialPosition(InitP) {}
};
// full creation of an observable entity
// creates an ogre::entity with an attached mesh
// this event is an extension upon a BasicWorldEntity, so contains all behaviour
// from CreateBasicWorldEntityEvent
struct CreateMeshWorldEntityEvent {
  std::string NodeName;
  Ogre::Vector3 InitialPosition;
  std::string MeshName;
  std::string EntityName;
  CreateMeshWorldEntityEvent(std::string NodeN, std::string MeshN,
                             std::string EntN, Ogre::Vector3 InitP)
      : NodeName(NodeN),
        MeshName(MeshN),
        EntityName(EntN),
        InitialPosition(InitP) {}
};

struct CreateGameObjectEvent {
  std::string NodeName;
  Ogre::Vector3 InitialPosition;
  std::string MeshName;
  std::string EntityName;
  Player* GamePlayer;
  int PlayerId;
  CreateGameObjectEvent(std::string NodeN, std::string MeshN, std::string EntN,
                        Ogre::Vector3 InitP, Player* P, int Id)
      : NodeName(NodeN),
        MeshName(MeshN),
        EntityName(EntN),
        InitialPosition(InitP),
        GamePlayer(P),
        PlayerId(Id) {}
};

struct CreateUnitProducingGameObjectEvent {
  std::string NodeName;
  Ogre::Vector3 InitialPosition;
  std::string MeshName;
  std::string EntityName;
  Player* GamePlayer;
  int UnitProdPerM;
  int PlayerId;
  CreateUnitProducingGameObjectEvent(std::string NodeN, std::string MeshN,
                                     std::string EntN, Ogre::Vector3 InitP,
                                     Player* P, int UPM, int Id)
      : NodeName(NodeN),
        MeshName(MeshN),
        EntityName(EntN),
        InitialPosition(InitP),
        GamePlayer(P),
        UnitProdPerM(UPM),
        PlayerId(Id) {}
};
struct CreateAttackingEntityEvent {
  std::string NodeName;
  Ogre::Vector3 InitialPosition;
  std::string MeshName;
  std::string EntityName;
  Player* GamePlayer;
  float Health;
  float Radius;
  float Power;
  int PlayerId;
  CreateAttackingEntityEvent(std::string NodeN, std::string MeshN,
                             std::string EntN, Ogre::Vector3 InitP, Player* P,
                             float H, float R, float Pwr, int Id)
      : NodeName(NodeN),
        MeshName(MeshN),
        EntityName(EntN),
        InitialPosition(InitP),
        GamePlayer(P),
        Health(H),
        Radius(R),
        Power(Pwr),
        PlayerId(Id) {}
};
struct ChangeGlobeVisibilityEvent {
  bool Visible;
  ChangeGlobeVisibilityEvent(bool Vis) : Visible(Vis) {}
};

struct CachedEntitiesReturnEvent {
  std::unordered_map<Ogre::SceneNode*, std::unordered_set<Ogre::SceneNode*>>
      Entities;
  CachedEntitiesReturnEvent(
      std::unordered_map<Ogre::SceneNode*, std::unordered_set<Ogre::SceneNode*>>
          Ents)
      : Entities(Ents) {}
};
