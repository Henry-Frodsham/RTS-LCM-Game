// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include "BillboardController.h"
#include "GlobeInterface.h"
#include "RenderEvent.h"

// owned by RenderSystem exclusively
// builds/updates a unit's health bar (a background + fill billboard) purely
// by calling BillboardController's generic primitives - has no Ogre scene
// access of its own, and BillboardController has no idea health bars exist
class HealthBarController {
 private:
  BillboardController* BillboardControl;
  GlobeInterface* GlobeInt;

 public:
  HealthBarController(BillboardController* BillboardControl,
                      GlobeInterface* GlobeInt);

  void CreateHealthBar(CreateHealthBarEvent Event);
  void UpdateHealthBar(UpdateHealthBarEvent Event);
};
