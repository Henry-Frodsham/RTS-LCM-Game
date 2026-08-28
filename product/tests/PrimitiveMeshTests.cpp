// Copyright (c) 2026 Henry Frodsham
#include <OGRE/Ogre.h>
#include <doctest/doctest.h>

#include <cmath>
#include <string>

#include "PrimitiveMeshFactory.h"
#include "RenderSystem.h"

// the shapes are built through RenderSystem::CreateEntity rather than by
// calling EnsureMesh directly - that is the path the game actually takes, and
// the SceneManager the builder needs belongs to RenderSystem
namespace {
Ogre::AxisAlignedBox BoundsAfterCreating(const std::string& MeshName,
                                         const std::string& EntityName) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.Init();
  Rs.CreateEntity(EntityName, MeshName);
  return Ogre::MeshManager::getSingleton().getByName(MeshName)->getBounds();
}

// every shape is built standing on y = 0 so it sits on the surface it was
// placed on rather than half sunk into it. the tolerance is for the 1%
// ogre pads a mesh bounding box by - the geometry itself starts at zero
bool SitsOnItsBase(const Ogre::AxisAlignedBox& Bounds) {
  const float Tolerance = Bounds.getSize().y * 0.02f;
  return std::abs(Bounds.getMinimum().y) < Tolerance;
}
}  // namespace

TEST_CASE("PrimitiveMesh - only its own names are primitives") {
  CHECK(PrimitiveMesh::IsPrimitiveName(PrimitiveMesh::kUnit));
  CHECK(PrimitiveMesh::IsPrimitiveName(PrimitiveMesh::kBoat));
  CHECK(PrimitiveMesh::IsPrimitiveName(PrimitiveMesh::kCity));
  CHECK_FALSE(PrimitiveMesh::IsPrimitiveName("Sphere.mesh"));
  CHECK_FALSE(PrimitiveMesh::IsPrimitiveName(""));
}

TEST_CASE("PrimitiveMesh - a unit is a tall tube with a nose along +Z") {
  const Ogre::AxisAlignedBox Bounds =
      BoundsAfterCreating(PrimitiveMesh::kUnit, "prim_test_unit");
  const Ogre::Vector3 Size = Bounds.getSize();

  // stands on the surface it was placed on, and is taller than the body is
  // wide - the whole point of the shape being 3d rather than a blit
  CHECK(SitsOnItsBase(Bounds));
  CHECK(Size.y > Size.x);

  // the nose cone reaches further forward than the tube it is mounted on, and
  // only forward - a symmetric bounding box would mean it had not been built
  CHECK(Bounds.getMaximum().z > Bounds.getMaximum().x);
  CHECK(Bounds.getMaximum().z > -Bounds.getMinimum().z);
}

TEST_CASE("PrimitiveMesh - a city is a tall square block") {
  const Ogre::AxisAlignedBox Bounds =
      BoundsAfterCreating(PrimitiveMesh::kCity, "prim_test_city");
  const Ogre::Vector3 Size = Bounds.getSize();

  CHECK(SitsOnItsBase(Bounds));
  CHECK(Size.x == doctest::Approx(Size.z));
  CHECK(Size.y > Size.x);
}

TEST_CASE("PrimitiveMesh - a boat is longer than it is wide, bow towards +Z") {
  const Ogre::AxisAlignedBox Bounds =
      BoundsAfterCreating(PrimitiveMesh::kBoat, "prim_test_boat");
  const Ogre::Vector3 Size = Bounds.getSize();

  CHECK(SitsOnItsBase(Bounds));
  CHECK(Size.z > Size.x);
  // the cabin carries the boat above hull height, so it reads from a way up
  CHECK(Size.y > Size.x * 0.5f);
}

TEST_CASE("PrimitiveMesh - building the same shape twice reuses one mesh") {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.Init();

  Rs.CreateEntity("prim_test_reuse_a", PrimitiveMesh::kUnit);
  const Ogre::AxisAlignedBox First =
      Ogre::MeshManager::getSingleton().getByName(PrimitiveMesh::kUnit)
          ->getBounds();

  CHECK_NOTHROW(Rs.CreateEntity("prim_test_reuse_b", PrimitiveMesh::kUnit));
  const Ogre::AxisAlignedBox Second =
      Ogre::MeshManager::getSingleton().getByName(PrimitiveMesh::kUnit)
          ->getBounds();

  CHECK(First.getMaximum() == Second.getMaximum());
}
