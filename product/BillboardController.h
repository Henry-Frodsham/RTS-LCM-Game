// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include "BillboardEvent.h"

// owned by RenderSystem exclusively
// generic, reusable creation/editing of world-space camera-facing
// billboards. knows nothing about what a billboard represents (health,
// floating labels, etc.) - see HealthBarController for a client built on
// top of this
class BillboardController {
 private:
  Ogre::SceneManager* SceneMngr;

 public:
  explicit BillboardController(Ogre::SceneManager* SceneManager);

  void CreateBillboardSet(CreateBillboardSetEvent Event);
  void AddBillboard(AddBillboardEvent Event);
  void EditBillboardDimensions(EditBillboardDimensionsEvent Event);
  void EditBillboardColour(EditBillboardColourEvent Event);
  void ChangeBillboardSetVisibility(ChangeBillboardSetVisibilityEvent Event);
};
