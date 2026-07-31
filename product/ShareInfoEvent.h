// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

// sharing info events, just for player general control to communicate selected
// entities or hit pos to interaction wheel
struct NotifySelectedEntity {
  Ogre::Entity* Entity;
  bool PressedTwice;
  NotifySelectedEntity(Ogre::Entity* E, bool PT)
      : Entity(E), PressedTwice(PT) {}
};

struct NotifyLatLonEvent {
  Ogre::Vector2f LatLon;
  explicit NotifyLatLonEvent(Ogre::Vector2f L) : LatLon(L) {}
};

struct NotifyPosEvent {
  Ogre::Vector3f Pos;
  explicit NotifyPosEvent(Ogre::Vector3f Position) : Pos(Position) {}
};

struct NotifySurfaceNormalEvent {
  Ogre::Vector3f SurfaceNormal;
  NotifySurfaceNormalEvent(Ogre::Vector3f SN) : SurfaceNormal(SN) {}
};
