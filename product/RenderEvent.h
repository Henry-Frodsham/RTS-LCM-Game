// Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <functional>
#include <string>

// unimplemented as of yet, render events will be used for instance threads to
// interact with ogre::root along with other RenderSystem owned ogre systems

struct TickEvent {
  float DeltaTime;
};

struct CreateSceneNodeEvent {
  std::reference_wrapper<Ogre::SceneNode*> Node;
  std::string NodeName;
  CreateSceneNodeEvent(Ogre::SceneNode*& N, std::string NodeN)
      : Node(N), NodeName(NodeN) {}
};

struct CreateOgreEntityEvent {
  std::reference_wrapper<Ogre::Entity*> Entity;
  std::string EntityName;
  std::string MeshName;
  CreateOgreEntityEvent(Ogre::Entity*& Ent, std::string EN, std::string MN)
      : Entity(Ent), EntityName(EN), MeshName(MN) {}
};

struct SetNodePositionEvent {
  std::reference_wrapper<Ogre::SceneNode*> NodeToChange;
  Ogre::Vector3 NewPosition;
  SetNodePositionEvent(Ogre::SceneNode*& N, Ogre::Vector3 Pos)
      : NodeToChange(N), NewPosition(Pos) {}
};

struct AttachEntityToScenNodeEvent {
  std::reference_wrapper<Ogre::Entity*> Entity;
  std::reference_wrapper<Ogre::SceneNode*> SceneNode;
  AttachEntityToScenNodeEvent(Ogre::Entity*& Ent, Ogre::SceneNode*& N)
      : Entity(Ent), SceneNode(N) {}
};
