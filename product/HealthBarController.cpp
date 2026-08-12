// Copyright (c) 2025 Henry Frodsham
#include "HealthBarController.h"

#include <algorithm>

namespace {
// bar dimensions as a fraction of globe radius, so they stay proportionate
// if the globe is ever resized (same convention as
// kPathPreviewHalfWidthFraction in RenderSystem.cpp). visual placeholders -
// tune once seen in-engine
constexpr float kHealthBarWidthFraction = 0.03f;
constexpr float kHealthBarHeightFraction = 0.006f;
// vertical clearance above the unit's own measured mesh height, as a
// fraction of that height - NOT a fraction of globe radius. the bar used to
// be offset by Radius * 0.02 above the unit's node origin, but a unit mesh's
// size has no fixed relationship to the globe's radius, so that offset
// landed the bar inside the mesh instead of above it. anchoring to the
// mesh's actual bounding-box top (see CreateHealthBar) fixes that regardless
// of how any given unit mesh is sized
constexpr float kHealthBarClearanceFraction = 0.15f;
// tiny separation between the background and fill layers so they don't
// z-fight while sitting on top of each other
constexpr float kHealthBarLayerSeparationFraction = 0.0005f;
}  // namespace

HealthBarController::HealthBarController(BillboardController* BillboardControl,
                                          GlobeInterface* GlobeInt)
    : BillboardControl(BillboardControl), GlobeInt(GlobeInt) {}

void HealthBarController::CreateHealthBar(CreateHealthBarEvent Event) {
  const float Radius = GlobeInt->GetGlobeRadius();
  const Ogre::Vector2 BarSize(Radius * kHealthBarWidthFraction,
                              Radius * kHealthBarHeightFraction);

  // top of the unit's mesh in the parent node's local space (RotateEntity-
  // ToSurfaceNormal rotates the node so local +Y is "up" off the globe
  // surface, and Entity::getBoundingBox() is already in that same local
  // space) - anchoring here instead of guessing keeps the bar clear of the
  // model no matter its shape
  const float UnitTop = Event.UnitEntity->getBoundingBox().getMaximum().y;
  const Ogre::Vector3 BgOffset(
      0.f, UnitTop * (1.f + kHealthBarClearanceFraction), 0.f);
  const Ogre::Vector3 FillOffset =
      BgOffset +
      Ogre::Vector3(0.f, Radius * kHealthBarLayerSeparationFraction, 0.f);

  Ogre::BillboardSet* BackgroundSet = nullptr;
  BillboardControl->CreateBillboardSet(CreateBillboardSetEvent(
      BackgroundSet, Event.ParentNode, BgOffset, "HEALTHBAR_BG", BarSize));
  Ogre::Billboard* Background = nullptr;
  BillboardControl->AddBillboard(
      AddBillboardEvent(Background, BackgroundSet, Ogre::Vector3::ZERO,
                        Ogre::ColourValue::Red, BarSize));

  Ogre::BillboardSet* FillSet = nullptr;
  BillboardControl->CreateBillboardSet(CreateBillboardSetEvent(
      FillSet, Event.ParentNode, FillOffset, "HEALTHBAR_FILL", BarSize));
  Ogre::Billboard* Fill = nullptr;
  BillboardControl->AddBillboard(AddBillboardEvent(
      Fill, FillSet, Ogre::Vector3::ZERO, Ogre::ColourValue::Green, BarSize));

  Event.OutSet.get() = FillSet;
  Event.OutFill.get() = Fill;
}

void HealthBarController::UpdateHealthBar(UpdateHealthBarEvent Event) {
  const float Ratio = std::clamp(Event.Ratio, 0.f, 1.f);
  const float Radius = GlobeInt->GetGlobeRadius();
  const float FullWidth = Radius * kHealthBarWidthFraction;
  const float Height = Radius * kHealthBarHeightFraction;

  BillboardControl->EditBillboardDimensions(EditBillboardDimensionsEvent(
      Event.Fill, Ogre::Vector2(FullWidth * Ratio, Height)));
}
