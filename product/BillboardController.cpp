// Copyright (c) 2025 Henry Frodsham
#include "BillboardController.h"

BillboardController::BillboardController(Ogre::SceneManager* SceneManager)
    : SceneMngr(SceneManager) {}

void BillboardController::CreateBillboardSet(CreateBillboardSetEvent Event) {
  Ogre::BillboardSet* Set = SceneMngr->createBillboardSet();
  Set->setMaterialName(Event.MaterialName);
  Set->setDefaultDimensions(Event.DefaultBillboardSize.x,
                            Event.DefaultBillboardSize.y);
  Set->setBillboardType(Ogre::BBT_POINT);

  Ogre::SceneNode* SetNode =
      Event.ParentNode->createChildSceneNode(Event.LocalOffset);
  SetNode->attachObject(Set);

  Event.OutSet.get() = Set;
}

void BillboardController::AddBillboard(AddBillboardEvent Event) {
  Ogre::Billboard* Board =
      Event.Set->createBillboard(Event.LocalPosition, Event.Colour);
  Board->setDimensions(Event.Dimensions.x, Event.Dimensions.y);

  Event.OutBillboard.get() = Board;
}

void BillboardController::EditBillboardDimensions(
    EditBillboardDimensionsEvent Event) {
  Event.Target->setDimensions(Event.NewDimensions.x, Event.NewDimensions.y);
}

void BillboardController::EditBillboardColour(
    EditBillboardColourEvent Event) {
  Event.Target->setColour(Event.NewColour);
}

void BillboardController::ChangeBillboardSetVisibility(
    ChangeBillboardSetVisibilityEvent Event) {
  Event.Set->setVisible(Event.Visible);
}
