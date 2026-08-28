// Copyright (c) 2025 Henry Frodsham
#include "PrimitiveMeshFactory.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace PrimitiveMesh {

const char kUnit[] = "PRIM_UNIT_MESH";
const char kBoat[] = "PRIM_BOAT_MESH";
const char kCity[] = "PRIM_CITY_MESH";

const char kMaterial[] = "PRIM_UNIT";
const char kSelectedMaterial[] = "PRIM_UNIT_SELECTED";

namespace {

// --- shape dimensions, all as fractions of the globe radius ---------------
// deliberately chunky and clearly taller than they are wide - these are read
// at a glance from an orbit camera, not admired up close. placeholders: tune
// once seen in engine, the same way the facing arrow fractions are

// unit: tube body with a cone nose
constexpr float kUnitBodyRadius = 0.006f;
constexpr float kUnitBodyHeight = 0.024f;
constexpr float kUnitNoseRadius = 0.005f;
constexpr float kUnitNoseLength = 0.016f;
// how far up the body the nose is mounted
constexpr float kUnitNoseHeightFraction = 0.62f;

// city: one rectangular block
constexpr float kCityHalfWidth = 0.016f;
constexpr float kCityHeight = 0.046f;

// boat: pointed hull plus a cabin block on the deck
constexpr float kBoatHalfLength = 0.024f;
constexpr float kBoatHalfWidth = 0.011f;
constexpr float kBoatHullHeight = 0.012f;
// where along the hull the bow starts tapering to its point, as a fraction of
// the half length
constexpr float kBoatShoulderFraction = 0.3f;
constexpr float kBoatCabinHalfWidth = 0.006f;
constexpr float kBoatCabinHeight = 0.016f;
constexpr float kBoatCabinSternZ = -0.016f;
constexpr float kBoatCabinBowZ = 0.002f;

// segment counts. low on purpose - a tube only has to read as round
constexpr int kTubeSegments = 14;
constexpr int kConeSegments = 12;

// emits one triangle with a flat face normal. everything here is written
// unindexed: the shapes are a few hundred vertices each, and sharing vertices
// would only mean splitting them apart again for their per-face normals
void AddTri(Ogre::ManualObject* Obj, const Ogre::Vector3& A,
            const Ogre::Vector3& B, const Ogre::Vector3& C) {
  Ogre::Vector3 Normal = (B - A).crossProduct(C - A);
  if (Normal.squaredLength() < 1e-20f) {
    return;
  }
  Normal.normalise();

  Obj->position(A);
  Obj->normal(Normal);
  Obj->position(B);
  Obj->normal(Normal);
  Obj->position(C);
  Obj->normal(Normal);
}

void AddQuad(Ogre::ManualObject* Obj, const Ogre::Vector3& A,
             const Ogre::Vector3& B, const Ogre::Vector3& C,
             const Ogre::Vector3& D) {
  AddTri(Obj, A, B, C);
  AddTri(Obj, A, C, D);
}

// signed area of an outline in the XZ plane. used only to work out which way
// round the caller happened to list its points, so the shape builders below
// can stay ignorant of the winding convention
float SignedArea(const std::vector<Ogre::Vector2>& Outline) {
  float Total = 0.f;
  for (size_t I = 0; I < Outline.size(); ++I) {
    const Ogre::Vector2& Current = Outline[I];
    const Ogre::Vector2& Next = Outline[(I + 1) % Outline.size()];
    Total += Current.x * Next.y - Next.x * Current.y;
  }
  return Total * 0.5f;
}

// extrudes a closed convex outline in the XZ plane (x in .x, z in .y) upwards
// into a capped solid. the outline may be given either way round - it is
// flipped here if needed so every face ends up wound outwards
void AddPrism(Ogre::ManualObject* Obj, std::vector<Ogre::Vector2> Outline,
              float BottomY, float TopY) {
  if (Outline.size() < 3) {
    return;
  }
  // a negative signed area is the order that puts the top cap's normal along
  // +Y, which is what the side and cap winding below assumes
  if (SignedArea(Outline) > 0.f) {
    std::reverse(Outline.begin(), Outline.end());
  }

  const size_t Count = Outline.size();
  std::vector<Ogre::Vector3> Bottom(Count);
  std::vector<Ogre::Vector3> Top(Count);
  for (size_t I = 0; I < Count; ++I) {
    Bottom[I] = Ogre::Vector3(Outline[I].x, BottomY, Outline[I].y);
    Top[I] = Ogre::Vector3(Outline[I].x, TopY, Outline[I].y);
  }

  for (size_t I = 0; I < Count; ++I) {
    const size_t Next = (I + 1) % Count;
    AddQuad(Obj, Bottom[I], Bottom[Next], Top[Next], Top[I]);
  }

  // caps, as fans off one corner. every outline here is convex, so a fan is
  // enough. the bottom fan walks the outline backwards to face downwards
  for (size_t I = 1; I + 1 < Count; ++I) {
    AddTri(Obj, Top[0], Top[I], Top[I + 1]);
    AddTri(Obj, Bottom[0], Bottom[Count - I], Bottom[Count - I - 1]);
  }
}

std::vector<Ogre::Vector2> RectOutline(float MinX, float MaxX, float MinZ,
                                       float MaxZ) {
  return {Ogre::Vector2(MinX, MinZ), Ogre::Vector2(MaxX, MinZ),
          Ogre::Vector2(MaxX, MaxZ), Ogre::Vector2(MinX, MaxZ)};
}

std::vector<Ogre::Vector2> CircleOutline(float Radius, int Segments) {
  std::vector<Ogre::Vector2> Outline;
  Outline.reserve(Segments);
  for (int I = 0; I < Segments; ++I) {
    const float Angle = Ogre::Math::TWO_PI * static_cast<float>(I) /
                        static_cast<float>(Segments);
    Outline.emplace_back(Radius * std::cos(Angle), Radius * std::sin(Angle));
  }
  return Outline;
}

// a closed cone with its base at BaseCentre and its tip Length away along
// Axis. unlike the prisms this one takes an arbitrary axis, because the
// unit's nose points sideways rather than up
void AddCone(Ogre::ManualObject* Obj, const Ogre::Vector3& BaseCentre,
             const Ogre::Vector3& Axis, float Radius, float Length,
             int Segments) {
  const Ogre::Vector3 Forward = Axis.normalisedCopy();
  const Ogre::Vector3 Side = Forward.perpendicular().normalisedCopy();
  const Ogre::Vector3 Up = Forward.crossProduct(Side).normalisedCopy();
  const Ogre::Vector3 Apex = BaseCentre + Forward * Length;

  std::vector<Ogre::Vector3> Ring(Segments);
  for (int I = 0; I < Segments; ++I) {
    const float Angle = Ogre::Math::TWO_PI * static_cast<float>(I) /
                        static_cast<float>(Segments);
    Ring[I] = BaseCentre + Side * (Radius * std::cos(Angle)) +
              Up * (Radius * std::sin(Angle));
  }

  for (int I = 0; I < Segments; ++I) {
    const int Next = (I + 1) % Segments;
    // (Side, Up, Forward) is right handed, so Ring runs anticlockwise about
    // Forward - listing a side face in ring order therefore winds it
    // outwards, and the base cap the other way round to face back down the
    // axis
    AddTri(Obj, Ring[I], Ring[Next], Apex);
    AddTri(Obj, Ring[Next], Ring[I], BaseCentre);
  }
}

void BuildUnit(Ogre::ManualObject* Obj, float Radius) {
  AddPrism(Obj, CircleOutline(Radius * kUnitBodyRadius, kTubeSegments), 0.f,
           Radius * kUnitBodyHeight);

  // the nose points along local +Z, the same forward the facing arrow uses
  // (see ComputeFacingOrientation in RenderSystem.cpp) - so with the unit's
  // node oriented by RotateEntityToSurfaceNormal, the body reads as pointing
  // wherever the unit is actually facing
  AddCone(
      Obj,
      Ogre::Vector3(0.f, Radius * kUnitBodyHeight * kUnitNoseHeightFraction,
                    0.f),
      Ogre::Vector3::UNIT_Z, Radius * kUnitNoseRadius,
      Radius * kUnitNoseLength, kConeSegments);
}

void BuildCity(Ogre::ManualObject* Obj, float Radius) {
  const float Half = Radius * kCityHalfWidth;
  AddPrism(Obj, RectOutline(-Half, Half, -Half, Half), 0.f,
           Radius * kCityHeight);
}

void BuildBoat(Ogre::ManualObject* Obj, float Radius) {
  const float HalfLength = Radius * kBoatHalfLength;
  const float HalfWidth = Radius * kBoatHalfWidth;
  const float ShoulderZ = HalfLength * kBoatShoulderFraction;

  // rectangular body with the bow drawn to a point at +Z, the same forward
  // the unit's nose cone uses
  const std::vector<Ogre::Vector2> Hull = {
      Ogre::Vector2(-HalfWidth, -HalfLength),
      Ogre::Vector2(HalfWidth, -HalfLength),
      Ogre::Vector2(HalfWidth, ShoulderZ),
      Ogre::Vector2(0.f, HalfLength),
      Ogre::Vector2(-HalfWidth, ShoulderZ)};
  AddPrism(Obj, Hull, 0.f, Radius * kBoatHullHeight);

  // cabin, sat on the deck towards the stern - carries most of the boat's
  // height so it is still picked out against the sea from a way up
  const float CabinHalf = Radius * kBoatCabinHalfWidth;
  AddPrism(Obj,
           RectOutline(-CabinHalf, CabinHalf, Radius * kBoatCabinSternZ,
                       Radius * kBoatCabinBowZ),
           Radius * kBoatHullHeight,
           Radius * (kBoatHullHeight + kBoatCabinHeight));
}

}  // namespace

bool IsPrimitiveName(const std::string& Name) {
  return Name == kUnit || Name == kBoat || Name == kCity;
}

void EnsureMesh(Ogre::SceneManager* SceneMngr, const std::string& Name,
                float GlobeRadius) {
  if (!SceneMngr || !IsPrimitiveName(Name)) {
    return;
  }
  if (Ogre::MeshManager::getSingleton().resourceExists(Name)) {
    return;
  }

  // a shape keeps whatever size it was first built at for the rest of the
  // session. rebuilding against a later globe radius would mean swapping the
  // mesh out from under every entity already holding it, and the only thing
  // that moves the radius is generating a whole new globe
  const float Radius = GlobeRadius > 0.f ? GlobeRadius : 1.f;

  Ogre::ManualObject* Builder = SceneMngr->createManualObject();
  Builder->begin(kMaterial, Ogre::RenderOperation::OT_TRIANGLE_LIST);

  if (Name == kUnit) {
    BuildUnit(Builder, Radius);
  } else if (Name == kBoat) {
    BuildBoat(Builder, Radius);
  } else {
    BuildCity(Builder, Radius);
  }

  Builder->end();
  // the mesh outlives the object that described it, and takes the bounds
  // ManualObject measured with it - which is what the health bar and facing
  // arrow anchor themselves against
  Builder->convertToMesh(Name);
  SceneMngr->destroyMovableObject(Builder);
}

}  // namespace PrimitiveMesh
