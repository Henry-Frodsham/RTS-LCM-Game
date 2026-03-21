#pragma once
#include <OGRE/Ogre.h>
struct NotifySelectedEntity {
  Ogre::Entity* Entity;
  NotifySelectedEntity(Ogre::Entity* E) : Entity(E) {}
};

struct NotifyLatLonEvent {
  Ogre::Vector2f LatLon;
  NotifyLatLonEvent(Ogre::Vector2f L) : LatLon(L) {}
};