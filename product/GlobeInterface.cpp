// Copyright (c) 2026 Henry Frodsham
#include "GlobeInterface.h"

#include <chrono>  // NOLINT(build/c++11)
#include <future>  // NOLINT(build/c++11)
#include <utility>

#include "RenderSystem.h"

namespace {
// the share of the build the tile work is worth, the rest being the vertex
// and index buffers. the two run back to back on the worker, so the bar has
// to be told which part of its travel each of them covers
constexpr float kTileGenerationShare = 0.9f;
}  // namespace

GlobeInterface::GlobeInterface()
    : CGlobe(nullptr),
      GlobeEntity(nullptr),
      GlobeSceneNode(nullptr),
      GlobeReporter(nullptr),
      GlobeConfig(nullptr),
      PendingGlobe(nullptr),
      GlobeVisible(false) {}

void GlobeInterface::Initialise() {
  GlobeReporter = new ErrorReporter();
  GlobeConfig = new ConfigManager("GlobeSettings", GlobeReporter);
}

// the worker is left entirely alone while it runs - the only thing this asks
// it is whether it is done, and the answer is acted on here so that every ogre
// call still happens on the render thread
void GlobeInterface::Update() {
  GlobeReporter->Dispatch();

  if (GenerationState.load() != GlobeGenerationState::Building) {
    return;
  }

  if (!GenerationTask.valid() ||
      GenerationTask.wait_for(std::chrono::seconds(0)) !=
          std::future_status::ready) {
    return;
  }

  FinaliseGeneration();
}

void GlobeInterface::ReloadConfiguration() {
  if (GlobeConfig) {
    GlobeConfig->LoadOrReload();
  }
}

// the settings are read here rather than on the worker, so the world that gets
// built is the one the options page had applied when play was pressed rather
// than whatever the file says by the time the thread gets around to it
void GlobeInterface::BeginGeneration(GenerateGlobeEvent Event) {
  // a globe is built once, when the game is entered. there is no route back to
  // the menu to change the settings and ask for another one yet, and rebuilding
  // under a world full of units standing on the old tiles is not a rebuild
  if (GenerationState.load() != GlobeGenerationState::NotBuilt) {
    return;
  }

  const unsigned int NumSubdivisions =
      GlobeConfig->GetValueOrDefault<unsigned int>("NumSubdivisions");
  const unsigned int CreationSeed =
      GlobeConfig->GetValueOrDefault<unsigned int>("CreationSeed");
  const float PlanetRadius =
      GlobeConfig->GetValueOrDefault<float>("PlanetRadius");

  PendingGlobe = new Globe();
  PendingGlobe->SetTransform(Ogre::Vector3::ZERO, PlanetRadius);

  GenerationProgress.store(0.f, std::memory_order_relaxed);
  GenerationState.store(GlobeGenerationState::Building);

  GenerationTask = std::async(
      std::launch::async, [this, NumSubdivisions, CreationSeed]() {
        PendingGlobe->Generate(
            NumSubdivisions, CreationSeed, [this](float Fraction) {
              GenerationProgress.store(Fraction * kTileGenerationShare,
                                       std::memory_order_relaxed);
            });

        PendingMeshData =
            PendingGlobe->BuildVisualMeshData([this](float Fraction) {
              GenerationProgress.store(
                  kTileGenerationShare +
                      (Fraction * (1.f - kTileGenerationShare)),
                  std::memory_order_relaxed);
            });
      });
}

float GlobeInterface::GetGenerationProgress() const {
  return GenerationProgress.load(std::memory_order_relaxed);
}

GlobeGenerationState GlobeInterface::GetGenerationState() const {
  return GenerationState.load();
}

// the worker's exceptions are rethrown by get(), so a failed build is reported
// the same way anything else here is rather than taking the process with it
void GlobeInterface::FinaliseGeneration() {
  try {
    GenerationTask.get();
  } catch (const std::exception& e) {
    delete PendingGlobe;
    PendingGlobe = nullptr;
    GenerationState.store(GlobeGenerationState::NotBuilt);

    GlobeReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::GLOBE_GENERATION_FAILED,
        fmt::format("the world could not be built \n original error: {}",
                    e.what())));
    return;
  }

  CGlobe = PendingGlobe;
  PendingGlobe = nullptr;

  Ogre::MeshPtr GlobeMesh = CGlobe->BuildVisualMesh(PendingMeshData);
  PendingMeshData = VisualMeshBufferData();

  RenderSystem& RS = RenderSystem::GetInstance();

  GlobeSceneNode = RS.CreateSceneNode("GlobeNode");
  GlobeEntity = RS.CreateEntity("GlobeEntity", GlobeMesh->getName());

  InitialiseGlobeMaterials();

  GlobeSceneNode->attachObject(GlobeEntity);
  GlobeSceneNode->setPosition(CGlobe->GetCenter());

  // the world is not on screen until the state that asked for it says so, and
  // it may already have said so while there was nothing to tell
  GlobeEntity->setVisible(GlobeVisible);

  GenerationProgress.store(1.f, std::memory_order_relaxed);
  GenerationState.store(GlobeGenerationState::Built);

  RS.RenderQueue->Enqueue(GlobeGeneratedEvent());
}

void GlobeInterface::InitialiseGlobeMaterials() {
  // reserve this area as a section to set up more complex materials in future
  GlobeEntity->setMaterialName("BaseWhiteNoLighting");
}

void GlobeInterface::ChangeGlobeVisibility(ChangeGlobeVisibilityEvent Event) {
  GlobeVisible = Event.Visible;

  if (GlobeEntity) {
    GlobeEntity->setVisible(GlobeVisible);
  }
}

Ogre::Vector3f GlobeInterface::GetGlobeCentre() const {
  return CGlobe ? CGlobe->GetCenter() : Ogre::Vector3::ZERO;
}

float GlobeInterface::GetGlobeRadius() const {
  return CGlobe ? CGlobe->GetRadius() : DefaultRadius;
}

GlobeRayHit GlobeInterface::CastRayFromWorld(const Ogre::Ray& WorldRay) const {
  if (!CGlobe) {
    return GlobeRayHit();
  }

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
