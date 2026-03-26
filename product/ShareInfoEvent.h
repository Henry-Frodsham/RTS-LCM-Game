#pragma once
#include <OGRE/Ogre.h>
struct NotifySelectedEntity {
  Ogre::Entity* Entity;
  bool PressedTwice;
  NotifySelectedEntity(Ogre::Entity* E, bool PT) : Entity(E), PressedTwice(PT) {}
};

struct NotifyLatLonEvent {
  Ogre::Vector2f LatLon;
  NotifyLatLonEvent(Ogre::Vector2f L) : LatLon(L) {}
};

struct NotifyPosEvent {
  Ogre::Vector3f Pos;
  NotifyPosEvent(Ogre::Vector3f Position) : Pos(Position) {}
};

