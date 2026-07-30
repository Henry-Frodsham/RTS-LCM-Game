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
