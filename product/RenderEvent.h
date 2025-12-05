// Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <string>

// unimplemented as of yet, render events will be used for instance threads to
// interact with ogre::root along with other RenderSystem owned ogre systems

struct TickEvent {
  float DeltaTime;
};
