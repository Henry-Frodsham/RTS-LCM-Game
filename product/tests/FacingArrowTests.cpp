// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <OGRE/Ogre.h>

#include <string>

#include "RenderEvent.h"
#include "RenderSystem.h"

// the facing arrow's whole job is to point somewhere and to keep looking like
// it points there from any camera angle. two earlier versions failed that by
// being flat: a flat shape has no thickness, so how much of it reaches the
// screen is decided by the angle it is seen from, and edge on there is
// nothing left of it at all.
//
// what can be checked without eyes on the screen is the geometry that
// property rests on - that the built mesh has real extent on all three axes,
// that it is longer than it is wide so it reads as pointing rather than as a
// blob, and that it is built in world space with a fixed orientation rather
// than anything that could be recomputed against a camera

namespace {
// builds one facing arrow through the render queue exactly as
// ECSHelper::CreateAndAddFacingArrowComponent does, and hands back the pieces
struct BuiltArrow {
  Ogre::SceneNode* Node = nullptr;
  Ogre::ManualObject* Object = nullptr;
};

BuiltArrow BuildArrow(const std::string& Name, Ogre::Vector3f Forward,
                      Ogre::Vector3f Up) {
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.Init();

  Ogre::SceneNode* Parent = RS.CreateSceneNode(Name + "_node");
  Ogre::Entity* UnitEntity = RS.CreateEntity(Name + "_ent", "unit.mesh");
  Parent->attachObject(UnitEntity);

  BuiltArrow Built;
  RS.RenderQueue->Enqueue(CreateFacingArrowEvent(
      std::ref(Built.Node), std::ref(Built.Object), Parent, UnitEntity, 1,
      Forward, Up));
  RS.RenderQueue->Dispatch();

  return Built;
}
}  // namespace

TEST_CASE("FacingArrow - the arrow is built with extent on all three axes, "
         "so it is a solid and not a flat shape") {
  BuiltArrow Built = BuildArrow("arrow_solid", Ogre::Vector3f(0.f, 0.f, 1.f),
                                Ogre::Vector3f(0.f, 1.f, 0.f));
  REQUIRE(Built.Object != nullptr);

  const Ogre::Vector3 Size = Built.Object->getBoundingBox().getSize();

  // the y extent is the one that matters. it is the thickness the two earlier
  // flat versions did not have, and the reason this one still has a
  // silhouette when the camera drops towards the horizon
  CHECK(Size.y > 0.f);
  CHECK(Size.x > 0.f);
  CHECK(Size.z > 0.f);
}

TEST_CASE("FacingArrow - the arrow is longer along the direction it points "
         "than it is wide across it") {
  BuiltArrow Built = BuildArrow("arrow_proportions",
                                Ogre::Vector3f(0.f, 0.f, 1.f),
                                Ogre::Vector3f(0.f, 1.f, 0.f));
  REQUIRE(Built.Object != nullptr);

  const Ogre::Vector3 Size = Built.Object->getBoundingBox().getSize();

  // local +Z is the facing direction (see ComputeFacingOrientation), so a
  // shape that is not clearly longest along z is not reading as an arrow
  CHECK(Size.z > Size.x);
  CHECK(Size.z > Size.y);
}

TEST_CASE("FacingArrow - the arrow starts at the unit and protrudes forwards "
         "rather than straddling it") {
  BuiltArrow Built = BuildArrow("arrow_protrudes",
                                Ogre::Vector3f(0.f, 0.f, 1.f),
                                Ogre::Vector3f(0.f, 1.f, 0.f));
  REQUIRE(Built.Object != nullptr);

  const Ogre::AxisAlignedBox Bounds = Built.Object->getBoundingBox();

  // built from z == 0 at the unit out to z == Length ahead of it, so all of
  // it lies forward of the mount point
  CHECK(Bounds.getMinimum().z == doctest::Approx(0.f));
  CHECK(Bounds.getMaximum().z > 0.f);

  // and centred on the mount height, not hanging off one side of it
  CHECK(Bounds.getMinimum().y == doctest::Approx(-Bounds.getMaximum().y));
}

TEST_CASE("FacingArrow - the arrow node holds its own world orientation "
         "rather than inheriting or tracking anything") {
  BuiltArrow Built = BuildArrow("arrow_static", Ogre::Vector3f(0.f, 0.f, 1.f),
                                Ogre::Vector3f(0.f, 1.f, 0.f));
  REQUIRE(Built.Node != nullptr);

  // an inherited or camera-relative orientation is exactly what would make
  // the arrow shift as the view moves. it is neither
  CHECK_FALSE(Built.Node->getInheritOrientation());

  const Ogre::Quaternion Before = Built.Node->getOrientation();
  RenderSystem::GetInstance().RenderQueue->Dispatch();
  CHECK(Built.Node->getOrientation() == Before);
}

TEST_CASE("FacingArrow - the arrow starts hidden, since nothing has selected "
         "the unit or attacked it yet") {
  BuiltArrow Built = BuildArrow("arrow_hidden", Ogre::Vector3f(0.f, 0.f, 1.f),
                                Ogre::Vector3f(0.f, 1.f, 0.f));
  REQUIRE(Built.Object != nullptr);

  CHECK_FALSE(Built.Object->isVisible());
}

TEST_CASE("FacingArrow - pointing the arrow a different way turns the node "
         "and leaves the geometry alone") {
  BuiltArrow North = BuildArrow("arrow_north", Ogre::Vector3f(0.f, 0.f, 1.f),
                                Ogre::Vector3f(0.f, 1.f, 0.f));
  BuiltArrow East = BuildArrow("arrow_east", Ogre::Vector3f(1.f, 0.f, 0.f),
                               Ogre::Vector3f(0.f, 1.f, 0.f));
  REQUIRE(North.Object != nullptr);
  REQUIRE(East.Object != nullptr);

  // the mesh is identical whichever way the unit faces - direction lives
  // entirely in the node's orientation, which is why UpdateFacingArrow-
  // OrientationEvent never has to touch geometry
  const Ogre::Vector3 NorthSize = North.Object->getBoundingBox().getSize();
  const Ogre::Vector3 EastSize = East.Object->getBoundingBox().getSize();
  CHECK(NorthSize.x == doctest::Approx(EastSize.x));
  CHECK(NorthSize.y == doctest::Approx(EastSize.y));
  CHECK(NorthSize.z == doctest::Approx(EastSize.z));

  CHECK_FALSE(North.Node->getOrientation() == East.Node->getOrientation());
}

TEST_CASE("FacingArrow - the node turns local +Z onto the direction the unit "
         "is facing") {
  // the orientation is the entire direction mechanism: the geometry is built
  // once pointing along local +Z and never rebuilt, so if the node does not
  // map +Z onto Forward the arrow points somewhere the unit is not going.
  // this went wrong silently for a long time - a left handed axis triple was
  // handed to Quaternion::FromAxes, which expects a proper rotation, and for
  // a unit facing +Z it degenerated all the way to the identity
  const Ogre::Vector3f Up(0.f, 1.f, 0.f);

  struct Case {
    const char* Name;
    Ogre::Vector3f Forward;
  };
  const Case Cases[] = {{"arrow_dir_pz", Ogre::Vector3f(0.f, 0.f, 1.f)},
                        {"arrow_dir_px", Ogre::Vector3f(1.f, 0.f, 0.f)},
                        {"arrow_dir_nz", Ogre::Vector3f(0.f, 0.f, -1.f)},
                        {"arrow_dir_nx", Ogre::Vector3f(-1.f, 0.f, 0.f)}};

  for (const Case& Current : Cases) {
    CAPTURE(Current.Name);
    BuiltArrow Built = BuildArrow(Current.Name, Current.Forward, Up);
    REQUIRE(Built.Node != nullptr);

    const Ogre::Vector3 Pointed =
        Built.Node->getOrientation() * Ogre::Vector3::UNIT_Z;

    CHECK(Pointed.x == doctest::Approx(Current.Forward.x).epsilon(0.001));
    CHECK(Pointed.y == doctest::Approx(Current.Forward.y).epsilon(0.001));
    CHECK(Pointed.z == doctest::Approx(Current.Forward.z).epsilon(0.001));
  }
}

TEST_CASE("FacingArrow - the node keeps local +Y on the surface normal, so "
         "the arrow lies along the ground rather than tilting off it") {
  const Ogre::Vector3f Up(0.f, 1.f, 0.f);
  BuiltArrow Built = BuildArrow("arrow_upright", Ogre::Vector3f(1.f, 0.f, 0.f),
                                Up);
  REQUIRE(Built.Node != nullptr);

  const Ogre::Vector3 Lifted =
      Built.Node->getOrientation() * Ogre::Vector3::UNIT_Y;

  CHECK(Lifted.x == doctest::Approx(Up.x).epsilon(0.001));
  CHECK(Lifted.y == doctest::Approx(Up.y).epsilon(0.001));
  CHECK(Lifted.z == doctest::Approx(Up.z).epsilon(0.001));
}

TEST_CASE("FacingArrow - a forward direction that is not already tangent to "
         "the surface is projected onto it rather than tilting the arrow") {
  const Ogre::Vector3f Up(0.f, 1.f, 0.f);
  // a forward vector with a large component straight up the surface normal,
  // which is what a unit walking towards a steep tile hands over
  BuiltArrow Built =
      BuildArrow("arrow_tilted", Ogre::Vector3f(1.f, 4.f, 0.f), Up);
  REQUIRE(Built.Node != nullptr);

  const Ogre::Vector3 Pointed =
      Built.Node->getOrientation() * Ogre::Vector3::UNIT_Z;

  // the vertical part is thrown away, leaving the tangent direction
  CHECK(Pointed.x == doctest::Approx(1.f).epsilon(0.001));
  CHECK(Pointed.y == doctest::Approx(0.f).epsilon(0.001));
  CHECK(Pointed.z == doctest::Approx(0.f).epsilon(0.001));
}
