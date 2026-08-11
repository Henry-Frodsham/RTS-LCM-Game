// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <functional>
#include <string>

// generic, reusable world-space billboard events, consumed by
// BillboardController. these carry no knowledge of what a billboard is being
// used for (health bars, floating labels, etc.) - see HealthBarController
// for a client that builds on top of these

// creates an empty Ogre::BillboardSet, offset from ParentNode by
// LocalOffset, using MaterialName and DefaultBillboardSize for any
// billboards subsequently added via AddBillboardEvent. OutSet is filled in
// synchronously since this runs on the render thread like every other
// render event
struct CreateBillboardSetEvent {
  std::reference_wrapper<Ogre::BillboardSet*> OutSet;
  Ogre::SceneNode* ParentNode;
  Ogre::Vector3 LocalOffset;
  std::string MaterialName;
  Ogre::Vector2 DefaultBillboardSize;
  CreateBillboardSetEvent(Ogre::BillboardSet*& OutS, Ogre::SceneNode* Parent,
                          Ogre::Vector3 Offset, std::string MatName,
                          Ogre::Vector2 DefaultSize)
      : OutSet(OutS),
        ParentNode(Parent),
        LocalOffset(Offset),
        MaterialName(MatName),
        DefaultBillboardSize(DefaultSize) {}
};

// adds a single billboard to a preexisting set, positioned at LocalPosition
// relative to the set's node
struct AddBillboardEvent {
  std::reference_wrapper<Ogre::Billboard*> OutBillboard;
  Ogre::BillboardSet* Set;
  Ogre::Vector3 LocalPosition;
  Ogre::ColourValue Colour;
  Ogre::Vector2 Dimensions;
  AddBillboardEvent(Ogre::Billboard*& OutB, Ogre::BillboardSet* S,
                    Ogre::Vector3 Pos, Ogre::ColourValue Col,
                    Ogre::Vector2 Dims)
      : OutBillboard(OutB),
        Set(S),
        LocalPosition(Pos),
        Colour(Col),
        Dimensions(Dims) {}
};

struct EditBillboardDimensionsEvent {
  Ogre::Billboard* Target;
  Ogre::Vector2 NewDimensions;
  EditBillboardDimensionsEvent(Ogre::Billboard* T, Ogre::Vector2 Dims)
      : Target(T), NewDimensions(Dims) {}
};

struct EditBillboardColourEvent {
  Ogre::Billboard* Target;
  Ogre::ColourValue NewColour;
  EditBillboardColourEvent(Ogre::Billboard* T, Ogre::ColourValue Col)
      : Target(T), NewColour(Col) {}
};

struct ChangeBillboardSetVisibilityEvent {
  Ogre::BillboardSet* Set;
  bool Visible;
  ChangeBillboardSetVisibilityEvent(Ogre::BillboardSet* S, bool Vis)
      : Set(S), Visible(Vis) {}
};
