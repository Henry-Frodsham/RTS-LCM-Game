// Copyright (c) 2026 Henry Frodsham
#include "GlobeInterface.h"
#include "RenderSystem.h"

GlobeInterface::GlobeInterface()
    : CGlobe(nullptr),
      GlobeEntity(nullptr),
      GlobeSceneNode(nullptr),
      GlobeReporter(nullptr) {}

void GlobeInterface::Initialise() { GlobeReporter = new ErrorReporter(); }

void GlobeInterface::Update() { GlobeReporter->Dispatch(); }
void GlobeInterface::GenerateGlobe(GlobeCreationConfiguration Config) {
  CGlobe = new Globe();
  CGlobe->Generate(Config.NumSubdivisions, Config.CreationSeed);
  Ogre::MeshPtr GlobeMesh = CGlobe->BuildVisualMesh();
  RenderSystem& RS = RenderSystem::GetInstance();

  GlobeSceneNode = RS.CreateSceneNode("GlobeNode");

  GlobeEntity = RS.CreateEntity("GlobeEntity", GlobeMesh->getName());

  InitialiseGlobeMaterials();

  GlobeSceneNode->attachObject(GlobeEntity);
  GlobeSceneNode->setPosition(CGlobe->GetCenter());
}

void GlobeInterface::InitialiseGlobeMaterials() {
  // reserve this area as a section to set up more complex materials in future
  GlobeEntity->setMaterialName("BaseWhiteNoLighting");
}

void GlobeInterface::ChangeGlobeVisibility(ChangeGlobeVisibilityEvent Event) {
  GlobeEntity->setVisible(Event.Visible);
}

GlobeRayHit GlobeInterface::CastRayFromWorld(const Ogre::Ray& WorldRay) const {
  // GlobeSceneNode is translated to CGlobe->GetCenter() in world space, and
  // Globe's own vertex positions are already generated in that same
  // Center-relative space (see Globe::BuildVisualMesh) - so we only need to
  // undo the node's world translation to get into the globe's local space.
  // Using _getDerivedPosition() rather than getPosition() so this still
  // works if the node is ever parented under something else later.
  const Ogre::Vector3 NodeWorldPos = GlobeSceneNode->_getDerivedPosition();

  const Ogre::Ray LocalRay(WorldRay.getOrigin() - NodeWorldPos,
                           WorldRay.getDirection());

  GlobeRayHit Hit = CGlobe->CastRay(LocalRay);
  if (Hit.DidHit) {
    Hit.HitPoint += NodeWorldPos;  // back to world space for the caller
  }
  return Hit;
}

uint32_t GlobeInterface::FindTileAtWorldPosition(
    const Ogre::Vector3& WorldPos) const {
  // same NodeWorldPos undo as CastRayFromWorld, so a world position resolves
  // to the same tile a ray-cast hit at that point would
  const Ogre::Vector3 NodeWorldPos = GlobeSceneNode->_getDerivedPosition();
  const Ogre::Vector3 LocalDir = WorldPos - NodeWorldPos - CGlobe->GetCenter();
  return CGlobe->FindTileAt(LocalDir);
}

Ogre::Vector3f GlobeInterface::GetWorldPositionForTile(uint32_t TileID) const {
  const Ogre::Vector3 NodeWorldPos = GlobeSceneNode->_getDerivedPosition();
  return CGlobe->GetTileSurfacePosition(TileID) + NodeWorldPos;
}

Ogre::Vector3f GlobeInterface::GetWorldNormalForTile(uint32_t TileID) const {
  return CGlobe->GetTileSurfaceNormal(TileID);
}