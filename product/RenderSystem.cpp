// Copyright (c) 2025 Henry Frodsham
#include "RenderSystem.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// anonymous namespace for entity selection
namespace {
bool RefineHit(const Ogre::Ray& WorldRay, Ogre::Entity* Ent,
               float* OutWorldDistance) {
  Ogre::SceneNode* Node = Ent->getParentSceneNode();
  if (!Node || !Node->isInSceneGraph()) return false;

  const auto World = Node->_getFullTransform();
  const auto Inverse = World.inverse();

  const Ogre::Vector3 LocalOrigin = Inverse * WorldRay.getOrigin();
  const Ogre::Vector3 LocalAhead =
      Inverse * (WorldRay.getOrigin() + WorldRay.getDirection());
  const Ogre::Ray LocalRay(LocalOrigin,
                           (LocalAhead - LocalOrigin).normalisedCopy());

  const std::pair<bool, Ogre::Real> Test =
      Ogre::Math::intersects(LocalRay, Ent->getBoundingBox());
  if (!Test.first) return false;

  const Ogre::Vector3 WorldHit = World * LocalRay.getPoint(Test.second);
  *OutWorldDistance = (WorldHit - WorldRay.getOrigin()).length();
  return true;
}

// destroys every MovableObject attached anywhere in Node's subtree before
// the node itself is torn down. SceneManager::destroySceneNode only frees
// the SceneNode objects, not whatever's attached to them or to their
// children - previously fine since units were a flat node+entity pair with
// no child nodes, but child-node-attached objects (e.g. a unit's health bar
// billboard set) would otherwise leak in the SceneManager
void DestroyAttachedMovablesRecursive(Ogre::SceneManager* SceneMngr,
                                      Ogre::SceneNode* Node) {
  for (Ogre::Node* Child : Node->getChildren()) {
    DestroyAttachedMovablesRecursive(SceneMngr,
                                     static_cast<Ogre::SceneNode*>(Child));
  }

  std::vector<Ogre::MovableObject*> Attached;
  for (size_t i = 0; i < Node->numAttachedObjects(); ++i)
    Attached.push_back(Node->getAttachedObject(i));
  Node->detachAllObjects();
  for (Ogre::MovableObject* Obj : Attached) SceneMngr->destroyMovableObject(Obj);
}

// half-width of the path preview ribbon, as a fraction of the globe radius -
// relative rather than a fixed world unit so it stays proportionate if the
// globe is ever resized
constexpr float kPathPreviewHalfWidthFraction = 0.01f;

// Ogre's manual-object line strips have no width control and render as a
// single-pixel hairline, which isn't visible enough for a move preview. this
// builds a flat ribbon (a triangle strip of quads lying on the surface)
// along the centreline instead, offsetting each point perpendicular to the
// path using the local "up" (radial direction from the globe centre) as an
// approximate surface normal
void AppendPathPreviewRibbon(Ogre::ManualObject* Obj,
                             const std::vector<Ogre::Vector3f>& Points,
                             const Ogre::Vector3f& GlobeCentre,
                             float HalfWidth) {
  const size_t Count = Points.size();
  for (size_t i = 0; i < Count; ++i) {
    Ogre::Vector3f Tangent;
    if (i == 0) {
      Tangent = Points[i + 1] - Points[i];
    } else if (i == Count - 1) {
      Tangent = Points[i] - Points[i - 1];
    } else {
      Tangent = Points[i + 1] - Points[i - 1];
    }
    if (Tangent.squaredLength() < 1e-12f) {
      Tangent = Ogre::Vector3f::UNIT_X;
    }
    Tangent.normalise();

    const Ogre::Vector3f Up = (Points[i] - GlobeCentre).normalisedCopy();
    Ogre::Vector3f Side = Tangent.crossProduct(Up);
    if (Side.squaredLength() < 1e-12f) {
      Side = Tangent.perpendicular();
    }
    Side.normalise();
    Side *= HalfWidth;

    Obj->position(Points[i] - Side);
    Obj->position(Points[i] + Side);
  }
}

// facing arrow dimensions, as fractions of globe radius - same convention as
// kPathPreviewHalfWidthFraction/kHealthBarWidthFraction, so it stays
// proportionate if the globe is ever resized. visual placeholders - tune
// once seen in-engine
constexpr float kFacingArrowLengthFraction = 0.025f;
constexpr float kFacingArrowHalfWidthFraction = 0.006f;
// vertical clearance above the unit's own measured mesh height, as a
// fraction of that height - same anchoring trick as
// kHealthBarClearanceFraction in HealthBarController.cpp
constexpr float kFacingArrowClearanceFraction = 0.2f;

// builds the world orientation that points local +Z at Forward while
// keeping local +Y aligned to Up. used for the facing arrow, which needs an
// explicit orientation of its own rather than inheriting its parent unit
// node's - RotateEntityToSurfaceNormal below only performs the minimal
// rotation from +Y to the surface normal, so the twist it leaves around
// that resulting axis is arbitrary and unrelated to movement direction
Ogre::Quaternion ComputeFacingOrientation(Ogre::Vector3f Forward,
                                          Ogre::Vector3f Up) {
  Up.normalise();
  Ogre::Vector3f Fwd = Forward - Up * Forward.dotProduct(Up);
  if (Fwd.squaredLength() < 1e-8f) {
    Fwd = Up.perpendicular();
  }
  Fwd.normalise();
  Ogre::Vector3f Right = Fwd.crossProduct(Up);
  Right.normalise();
  Up = Right.crossProduct(Fwd);

  Ogre::Quaternion Orientation;
  Orientation.FromAxes(Right, Up, Fwd);
  return Orientation;
}
}  // namespace

RenderSystem::RenderSystem()
    : OgreRoot(nullptr),
      SceneManager(nullptr),
      PrimaryWindow(nullptr),
      OverlaySystem(nullptr),
      OverlayControl(nullptr),
      BillboardControl(nullptr),
      HealthBarControl(nullptr),
      DefaultViewPort(nullptr),
      SDLWindow(nullptr),
      ViewPortListener(nullptr),
      GlobeInt(nullptr),
      PathPreviewLine(nullptr),
      RaySceneQuery(nullptr),
      ViewPorts(NULL),
      RenderErrorReporter(ErrorReporter()) {
  RenderBus = new EventBus();
  // only publishes to render bus so why bother specifying the bus each time
  RenderQueue = new EventQueue(RenderBus);

  DeltaTime = 1.f;

  LastFrameTime = std::chrono::high_resolution_clock::now();

  RenderConfig = new ConfigManager("VideoSettings", &RenderErrorReporter);
}
RenderSystem::~RenderSystem() {
  if (SDLWindow) {
    SDL_DestroyWindow(SDLWindow);
  }
  SDL_Quit();

  // shutdown, destroy root and SceneMgr
}
// main render loop, run regardless of state to maintain responsiveness
void RenderSystem::RenderFrame() {
  auto FrameStart = std::chrono::high_resolution_clock::now();

  std::chrono::steady_clock::time_point CurrentTime =
      std::chrono::high_resolution_clock::now();

  DeltaTime = std::chrono::duration<float>(FrameStart - LastFrameTime).count();

  LastFrameTime = FrameStart;

  // dispatch internal queue aswell
  RenderErrorReporter.Dispatch();
  // dispatch overlay controller... etc
  UpdateExclusiveHandlers();

  if (PrimaryWindow && !PrimaryWindow->isClosed()) {
    Ogre::WindowEventUtilities::messagePump();
    RenderQueue->Dispatch();
    GlobeInt->Update();

    OgreRoot->renderOneFrame();
  } else {
    // window closed, shutdown app
    RenderErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::RENDER_WINDOW_CLOSED));
  }
}

// creates Ogre3d root and creates primary render window
void RenderSystem::Init() {
  if (IsInit) {
    return;
  }
  SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
    // sdl failed init, throw fatal
    RenderErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::SDL_FAILED_INIT));
  }

#ifdef _DEBUG
  OgreRoot = new Ogre::Root("plugins_d.cfg", "", "ogre.log");
#else
  OgreRoot = new Ogre::Root("plugins.cfg", "", "ogre.log");
#endif

  const Ogre::RenderSystemList& RenderSystems =
      OgreRoot->getAvailableRenderers();
  auto SharedParams =
      Ogre::GpuProgramManager::getSingleton().createSharedParameters(
          "ViewerParams");
  SharedParams->addConstantDefinition("viewingPlayerID", Ogre::GCT_FLOAT1);
  SharedParams->setNamedConstant("viewingPlayerID", 0.0f);
  if (RenderSystems.size() == 0) {
    RenderErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::OGRE_NO_AVAILABLE_RENDER_SYSTEM));
  }
  OgreRoot->setRenderSystem(RenderSystems[0]);

  OgreRoot->initialise(false);

  RenderWindowSettings = Ogre::RenderWindowDescription(
      RenderConfig->GetValueOrDefault<std::string>("WindowName"),
      RenderConfig->GetValueOrDefault<unsigned int>("WindowWidth"),
      RenderConfig->GetValueOrDefault<unsigned int>("WindowHeight"),
      RenderConfig->GetValueOrDefault<bool>("FullScreen"));

  PrimaryWindow = OgreRoot->createRenderWindow(RenderWindowSettings);

  ViewPortListener = new ViewPortUpdateListener();

  PrimaryWindow->addListener(ViewPortListener);
  // get the window handle to bind with SDL
  size_t WindowHandle = 0;
  PrimaryWindow->getCustomAttribute("WINDOW", &WindowHandle);

  SDLWindow = SDL_CreateWindowFrom(reinterpret_cast<void*>(WindowHandle));
  if (!SDLWindow) {
    RenderErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::SDL_FAILED_BIND));
  }
  if (RenderConfig->GetValueOrDefault<bool>("FullScreen")) {
#ifdef _WIN32
    while (::ShowCursor(TRUE) < 0);  // NOLINT [whitespace/empty_loop_body]
    ::SetCursor(
        ::LoadCursor(NULL, IDC_ARROW));  // NOLINT [whitespace/end_of_line]
#endif
  }
  SDL_SetRelativeMouseMode(SDL_FALSE);
  SDL_SetWindowGrab(SDLWindow, SDL_FALSE);
  SceneManager = OgreRoot->createSceneManager();
  Ogre::MovableObject::setDefaultQueryFlags(0);
  SceneManager->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));

  OverlaySystem = new Ogre::OverlaySystem();
  SceneManager->addRenderQueueListener(OverlaySystem);

  InitBasicResourceGroups();

  GlobeInt = new GlobeInterface();

  GlobeInt->Initialise();
  GlobeInt->GenerateGlobe();

  OverlayControl = new OverlayController();

  BillboardControl = new BillboardController(SceneManager);

  HealthBarControl = new HealthBarController(BillboardControl, GlobeInt);

  DefaultViewPort = CreateViewPort();

  DefaultViewPort->setOverlaysEnabled(true);

  RaySceneQuery = SceneManager->createRayQuery(Ogre::Ray());
  RaySceneQuery->setSortByDistance(true);

  InitRenderResponsibility();
  IsInit = true;
}

// add a new camera to the game world, usually indicates a new split screen
// instance but isnt exclusive
ViewPortController* RenderSystem::CreateViewPort() {
  Ogre::SceneManager::CameraList CameraList = SceneManager->getCameras();

  Ogre::Camera* Camera =
      SceneManager->createCamera(std::to_string(CameraList.size()));

  // clip planes are expressed as a fraction of the globe's radius rather
  // than flat literals, so they stay correct however small/large the
  // configured planet size is instead of assuming a unit-radius globe
  constexpr float kNearClipRadiusFraction = 0.1f;
  constexpr float kFarClipRadiusFraction = 1000.0f;
  const float GlobeRadius = GlobeInt->GetGlobeRadius();
  Camera->setNearClipDistance(GlobeRadius * kNearClipRadiusFraction);
  Camera->setFarClipDistance(GlobeRadius * kFarClipRadiusFraction);

  Camera->setAutoAspectRatio(true);

  // configure a scene node for the camera to be able to be moved
  Ogre::SceneNode* CameraNode =
      SceneManager->getRootSceneNode()->createChildSceneNode(
          "CameraNode_" + std::to_string(CameraList.size()));

  CameraNode->attachObject(Camera);

  Ogre::Viewport* AddedViewPort = PrimaryWindow->addViewport(
      Camera, CameraList.size());  // temporary Z order

  ViewPortController* newController = new ViewPortController(AddedViewPort);
  const float SurfaceRadius =
      GlobeRadius * 1.05f;  // matches kHeightScale headroom
  newController->SetOrbitDistanceLimits(
      SurfaceRadius * 2.f,    // never clip terrain
      SurfaceRadius * 6.0f);  // arbitrary max zoom-out
  newController->InitOrbitCamera(GlobeInt->GetGlobeCentre(),
                                 SurfaceRadius * 1.12f);  // low orbit

  ViewPorts.push_back(newController);

  ScaleViewPorts();

  return newController;
}

void RenderSystem::ScaleViewPorts() {
  int NumberToScale = ViewPorts.size();
  for (int I = 0; I < NumberToScale; I++) {
    ViewPortController* VPC = ViewPorts.at(I);

    // the amount of relative screen size per viewport to add
    // each viewPort gets an equal portion
    float SizePerVP =
        1.f / float(NumberToScale);  // NOLINT [readability/casting]

    VPC->ChangeViewPortDimensions(SizePerVP * float(I), 0.f, SizePerVP,
                                  1.f);  // NOLINT [readability/casting]
  }
}

// delegates render responsibility to other classes
// think of e.g OverlayController as an arm and RenderSystem as the body
void RenderSystem::InitRenderResponsibility() {
  // overlay controller
  RenderBus->Subscribe<OverlayAddBoxEvent>(std::bind(
      &OverlayController::AddBox, OverlayControl, std::placeholders::_1));
  RenderBus->Subscribe<OverlayAddTextEvent>(std::bind(
      &OverlayController::AddText, OverlayControl, std::placeholders::_1));
  RenderBus->Subscribe<OverlayEditPanelEvent>(std::bind(
      &OverlayController::EditPanel, OverlayControl, std::placeholders::_1));
  RenderBus->Subscribe<OverlayEditTextEvent>(std::bind(
      &OverlayController::EditText, OverlayControl, std::placeholders::_1));
  RenderBus->Subscribe<CreateOverlayEvent>(
      std::bind(&OverlayController::CreateOverlay, OverlayControl,
                std::placeholders::_1));
  RenderBus->Subscribe<ChangeOverlayVisibilityEvent>(
      std::bind(&OverlayController::ChangeOverlayVisibility, OverlayControl,
                std::placeholders::_1));
  RenderBus->Subscribe<CursorMovementEvent>(
      std::bind(&OverlayController::OverlayCursorCheck, OverlayControl,
                std::placeholders::_1));
  RenderBus->Subscribe<PressActionCommand>(
      std::bind(&OverlayController::OverlayPressedCheck, OverlayControl,
                std::placeholders::_1));
  RenderBus->Subscribe<RegisterOnPressCallBackEvent>(
      std::bind(&OverlayController::RegisterOnPressCallBack, OverlayControl,
                std::placeholders::_1));
  RenderBus->Subscribe<OverlayAddTextToPanelEvent>(
      std::bind(&OverlayController::AddTextToPanel, OverlayControl,
                std::placeholders::_1));
  RenderBus->Subscribe<OverlayAddBoxToPanelEvent>(
      std::bind(&OverlayController::AddBoxToPanel, OverlayControl,
                std::placeholders::_1));

  // billboard controller - generic, reusable world-space billboards
  RenderBus->Subscribe<CreateBillboardSetEvent>(
      std::bind(&BillboardController::CreateBillboardSet, BillboardControl,
                std::placeholders::_1));
  RenderBus->Subscribe<AddBillboardEvent>(std::bind(
      &BillboardController::AddBillboard, BillboardControl,
      std::placeholders::_1));
  RenderBus->Subscribe<EditBillboardDimensionsEvent>(
      std::bind(&BillboardController::EditBillboardDimensions,
                BillboardControl, std::placeholders::_1));
  RenderBus->Subscribe<EditBillboardColourEvent>(
      std::bind(&BillboardController::EditBillboardColour, BillboardControl,
                std::placeholders::_1));
  RenderBus->Subscribe<ChangeBillboardSetVisibilityEvent>(
      std::bind(&BillboardController::ChangeBillboardSetVisibility,
                BillboardControl, std::placeholders::_1));

  // health bar controller - built on top of BillboardController
  RenderBus->Subscribe<CreateHealthBarEvent>(
      std::bind(&HealthBarController::CreateHealthBar, HealthBarControl,
                std::placeholders::_1));
  RenderBus->Subscribe<UpdateHealthBarEvent>(
      std::bind(&HealthBarController::UpdateHealthBar, HealthBarControl,
                std::placeholders::_1));

  // core Ogre interactions
  RenderBus->Subscribe<CreateSceneNodeEvent>(std::bind(
      &RenderSystem::CreateSceneNodeFromEvent, this, std::placeholders::_1));
  RenderBus->Subscribe<CreateOgreEntityEvent>(std::bind(
      &RenderSystem::CreateEntityFromEvent, this, std::placeholders::_1));
  RenderBus->Subscribe<SetNodePositionEvent>(std::bind(
      &RenderSystem::SetNodePositionFromEvent, this, std::placeholders::_1));
  RenderBus->Subscribe<AttachEntityToScenNodeEvent>(std::bind(
      &RenderSystem::AttachEntityToNodeFromEvent, this, std::placeholders::_1));
  RenderBus->Subscribe<StartRayTraceEvent>(std::bind(
      &RenderSystem::AssembleRayTraceEvent, this, std::placeholders::_1));
  RenderBus->Subscribe<ScaleEntityEvent>(std::bind(
      &RenderSystem::ScaleEntityFromEvent, this, std::placeholders::_1));
  RenderBus->Subscribe<SetEntPositionEvent>(std::bind(
      &RenderSystem::SetEntPosFromEvent, this, std::placeholders::_1));
  RenderBus->Subscribe<RotateEntToSurfaceNormalEvent>(std::bind(
      &RenderSystem::RotateEntityToSurfaceNormal, this, std::placeholders::_1));
  RenderBus->Subscribe<ChangeEntMaterialEvent>(std::bind(
      &RenderSystem::ChangeEntityMaterial, this, std::placeholders::_1));
  RenderBus->Subscribe<AddOwnerShipToEntEvent>(
      std::bind(&RenderSystem::AddOwnerShipToEnt, this, std::placeholders::_1));
  RenderBus->Subscribe<DestroyNodeEvent>(
      std::bind(&RenderSystem::DestroyNode, this, std::placeholders::_1));
  RenderBus->Subscribe<DestroyEntityEvent>(
      std::bind(&RenderSystem::DestroyEntity, this, std::placeholders::_1));
  RenderBus->Subscribe<RevalEntityRangeEvent>(
      std::bind(&RenderSystem::EntityRangeCheck, this, std::placeholders::_1));
  RenderBus->Subscribe<CreateFacingArrowEvent>(std::bind(
      &RenderSystem::CreateFacingArrow, this, std::placeholders::_1));
  RenderBus->Subscribe<UpdateFacingArrowOrientationEvent>(
      std::bind(&RenderSystem::UpdateFacingArrowOrientation, this,
                std::placeholders::_1));
  RenderBus->Subscribe<ChangeFacingArrowVisibilityEvent>(
      std::bind(&RenderSystem::ChangeFacingArrowVisibility, this,
                std::placeholders::_1));
  RenderBus->Subscribe<ChangeCameraOrbitAngleEvent>(
      std::bind(&RenderSystem::ChangeCameraOrbit, this, std::placeholders::_1));
  RenderBus->Subscribe<ChangeCameraOrbitDepthEvent>(
      std::bind(&RenderSystem::ChangeCameraDepth, this, std::placeholders::_1));
  RenderBus->Subscribe<UpdatePathPreviewEvent>(
      std::bind(&RenderSystem::UpdatePathPreview, this, std::placeholders::_1));

  // view port update events
  RenderBus->Subscribe<RegisterOverlayToViewPortEvent>(
      std::bind(&ViewPortUpdateListener::AssignOverlayToViewport,
                ViewPortListener, std::placeholders::_1));
  // globe events
  RenderBus->Subscribe<ChangeGlobeVisibilityEvent>(std::bind(
      &GlobeInterface::ChangeGlobeVisibility, GlobeInt, std::placeholders::_1));
}

// initialise the basic resources (not game textures and mats etc)
// complex meshes and material resource groups are managed by World
// the resource groups here are basic solid colours for UI
void RenderSystem::InitBasicResourceGroups() {
  // no need to store a ptr, only used here
  Ogre::ResourceGroupManager& Rgm = Ogre::ResourceGroupManager::getSingleton();

  Rgm.createResourceGroup("Overlay");
  Rgm.createResourceGroup("Font");
  // solid colours for overlay
  std::filesystem::path PathSolDir = std::filesystem::current_path();
  Rgm.addResourceLocation(
      PathSolDir.append(std::string("resources\\simple\\mat\\"))
          .generic_string(),
      "FileSystem", "Overlay");
  PathSolDir = std::filesystem::current_path();
  Rgm.addResourceLocation(
      PathSolDir.append(std::string("resources\\simple\\mesh"))
          .generic_string(),
      "FileSystem", "Mesh");
  PathSolDir = std::filesystem::current_path();
  Rgm.addResourceLocation(
      PathSolDir.append(std::string("resources\\font\\")).generic_string(),
      "FileSystem", "Font");
  PathSolDir = std::filesystem::current_path();
  Rgm.addResourceLocation(
      PathSolDir.append(std::string("resources\\globe\\mat")).generic_string(),
      "FileSystem", "Font");
  PathSolDir = std::filesystem::current_path();
  Rgm.addResourceLocation(
      PathSolDir.append(std::string("resources\\globe\\mesh")).generic_string(),
      "FileSystem", "Font");
  Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Overlay");
  Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Mesh");
  Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Font");
}

void RenderSystem::UpdateExclusiveHandlers() { OverlayControl->ParentUpdate(); }
float RenderSystem::GetDeltaTime() { return DeltaTime; }
SDL_Window* RenderSystem::GetSDLWindow() { return SDLWindow; }

Ogre::RenderWindowDescription RenderSystem::GetPrimaryWindowInformation() {
  return RenderWindowSettings;
}

Ogre::SceneNode* RenderSystem::CreateSceneNode(std::string Name) {
  return SceneManager->getRootSceneNode()->createChildSceneNode(Name);
}

Ogre::SceneNode* RenderSystem::GetSceneNodeFromName(std::string Name) {
  try {
    return SceneManager->getSceneNode(Name);
  } catch (std::exception e) {
    RenderErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::SCENE_NODE_DOESNT_EXIST,
        fmt::format("the queried scene node {} didnt exist", Name)));
    return nullptr;
  }
}

ViewPortController* RenderSystem::GetPrimaryViewport() {
  return DefaultViewPort;
}

Ogre::Entity* RenderSystem::CreateEntity(std::string Name,
                                         std::string MeshName) {
  return SceneManager->createEntity(Name, MeshName);
}

void RenderSystem::CreateSceneNodeFromEvent(CreateSceneNodeEvent Event) {
  try {
    Event.Node.get() =
        SceneManager->getRootSceneNode()->createChildSceneNode(Event.NodeName);
  } catch (Ogre::ItemIdentityException) {
    Event.Node.get() = SceneManager->getSceneNode(Event.NodeName);
  }
}
void RenderSystem::CreateEntityFromEvent(CreateOgreEntityEvent Event) {
  Ogre::Entity* Ent =
      SceneManager->createEntity(Event.EntityName, Event.MeshName);

  Ent->setQueryFlags(Event.QueryFlags);

  Ent->getUserObjectBindings().setUserAny(
      "EnttHandle",
      Ogre::Any(static_cast<uint32_t>(entt::to_integral(Event.OwningEntity))));

  Event.Entity.get() = Ent;
}
void RenderSystem::SetNodePositionFromEvent(SetNodePositionEvent Event) {
  Event.NodeToChange.get()->setPosition(Event.NewPosition);
}
void RenderSystem::RotateEntityToSurfaceNormal(
    RotateEntToSurfaceNormalEvent Event) {
  Ogre::SceneNode* UnitSN = Event.Entity->getParentSceneNode();

  // Event.SurfaceNormal now comes straight from GlobeRayHit::SurfaceNormal,
  // i.e. the actual face normal of the triangle that was hit - this already
  // accounts for per-tile elevation, unlike the old sphere-radial normal.
  Ogre::Quaternion Rotation =
      Ogre::Vector3::UNIT_Y.getRotationTo(Event.SurfaceNormal);
  UnitSN->setOrientation(Rotation);
}
void RenderSystem::SetEntPosFromEvent(SetEntPositionEvent Event) {
  Ogre::SceneNode* SN = Event.Ent->getParentSceneNode();
  SN->setPosition(Event.Vec);
}
void RenderSystem::AttachEntityToNodeFromEvent(
    AttachEntityToScenNodeEvent Event) {
  Event.SceneNode.get()->attachObject(Event.Entity.get());
}
void RenderSystem::ScaleEntityFromEvent(ScaleEntityEvent Event) {
  Ogre::Entity* Ent = SceneManager->getEntity(Event.EntName);
  Ogre::SceneNode* Node = Ent->getParentSceneNode();
  // to make my life easier ill just be scaling every axis
  Node->scale(Event.NewScale, Event.NewScale, Event.NewScale);
}
void RenderSystem::DestroyNode(DestroyNodeEvent Event) {
  if (!Event.NodeToDestroy) return;

  DestroyAttachedMovablesRecursive(SceneManager, Event.NodeToDestroy);

  Event.NodeToDestroy->removeAndDestroyAllChildren();

  SceneManager->destroySceneNode(Event.NodeToDestroy);
}
void RenderSystem::DestroyEntity(DestroyEntityEvent Event) {
  if (!Event.EntityToDestroy) return;

  Ogre::SceneNode* NodeToDestroy = Event.EntityToDestroy->getParentSceneNode();
  if (!NodeToDestroy) return;

  DestroyAttachedMovablesRecursive(SceneManager, NodeToDestroy);

  NodeToDestroy->removeAndDestroyAllChildren();

  SceneManager->destroySceneNode(NodeToDestroy);
}

void RenderSystem::EntityRangeCheck(RevalEntityRangeEvent Event) {
  std::unordered_set<Ogre::SceneNode*> EntitiesInRange;

  Ogre::SceneNode* Node = Event.EntToCheck->getParentSceneNode();
  if (!Node) return;
  if (!Node->isInSceneGraph()) return;

  try {
    Ogre::SphereSceneQuery* Query = SceneManager->createSphereQuery(
        Ogre::Sphere(Node->_getDerivedPosition(), Event.GeneralRange));
    Ogre::SceneQueryResult& Result = Query->execute();

    for (Ogre::MovableObject* Movable : Result.movables) {
      Ogre::SceneNode* TargetNode = Movable->getParentSceneNode();
      if (TargetNode && TargetNode != Node) {
        EntitiesInRange.insert(TargetNode);
      }
    }

    SceneManager->destroyQuery(Query);
  } catch (std::exception& e) {
    RenderErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::RANGE_CHECK_FAILED));
  }
  Event.CallBackQueue->Enqueue(
      EntitiesInRangeUpdateEvent(Node, EntitiesInRange));
}

void RenderSystem::CreateFacingArrow(CreateFacingArrowEvent Event) {
  const float Radius = GlobeInt->GetGlobeRadius();
  const float Length = Radius * kFacingArrowLengthFraction;
  const float HalfWidth = Radius * kFacingArrowHalfWidthFraction;

  // anchor above the unit mesh's own measured bounding box, same reasoning
  // as HealthBarController::CreateHealthBar - a fixed globe-radius-relative
  // offset would land inside some unit meshes and floating above others
  const float UnitTop = Event.UnitEntity->getBoundingBox().getMaximum().y;
  const Ogre::Vector3 LocalOffset(
      0.f, UnitTop * (1.f + kFacingArrowClearanceFraction), 0.f);

  Ogre::SceneNode* ArrowNode = Event.ParentNode->createChildSceneNode(LocalOffset);
  // position still inherits from ParentNode, but orientation is fully our
  // own - see ComputeFacingOrientation for why
  ArrowNode->setInheritOrientation(false);
  ArrowNode->setOrientation(
      ComputeFacingOrientation(Event.WorldForward, Event.WorldUp));

  Ogre::ManualObject* Arrow = SceneManager->createManualObject();
  Arrow->begin("FACING_ARROW", Ogre::RenderOperation::OT_TRIANGLE_LIST);
  // a simple triangle pointing along local +Z, flat in the local XZ plane
  Arrow->position(-HalfWidth, 0.f, 0.f);
  Arrow->position(HalfWidth, 0.f, 0.f);
  Arrow->position(0.f, 0.f, Length);
  Arrow->end();

  // same per-section custom param unit materials read off SubEntity (see
  // AddOwnerShipToEnt) - lets FACING_ARROW.material discard the arrow on
  // every viewport except the owning player's, same mechanism as
  // RED_UNIT/WHITE and PATH_PREVIEW
  Arrow->getSection(0)->setCustomParameter(
      0, Ogre::Vector4(static_cast<float>(Event.OwnerID), 0.0f, 0.0f, 0.0f));

  ArrowNode->attachObject(Arrow);
  ArrowNode->setVisible(false);

  Event.OutNode.get() = ArrowNode;
  Event.OutObject.get() = Arrow;
}

void RenderSystem::UpdateFacingArrowOrientation(
    UpdateFacingArrowOrientationEvent Event) {
  if (!Event.ArrowNode) return;
  Event.ArrowNode->setOrientation(
      ComputeFacingOrientation(Event.WorldForward, Event.WorldUp));
}

void RenderSystem::ChangeFacingArrowVisibility(
    ChangeFacingArrowVisibilityEvent Event) {
  if (!Event.ArrowNode) return;
  Event.ArrowNode->setVisible(Event.Visible);
}

void RenderSystem::AssembleRayTraceEvent(StartRayTraceEvent Event) {
  ViewPortController* RayVPC = FindViewPortFromDevice(Event.Device);
  Ogre::Ray WorldRay = RayVPC->GetWorldRayForDevice(Event);

  GlobeRayHit Hit = GlobeInt->CastRayFromWorld(WorldRay);

  RayPickResult Pick;
  Pick.Terrain = Hit;

  RaySceneQuery->setRay(WorldRay);
  RaySceneQuery->setQueryMask(RenderQueryFlags::kSelectableUnit);
  Ogre::RaySceneQueryResult& Results = RaySceneQuery->execute();

  float BestDistance = std::numeric_limits<float>::max();
  for (const Ogre::RaySceneQueryResultEntry& Entry : Results) {
    Ogre::Entity* Candidate = dynamic_cast<Ogre::Entity*>(Entry.movable);
    if (!Candidate) continue;

    float Distance = 0.f;
    if (!RefineHit(WorldRay, Candidate, &Distance)) continue;
    if (Distance >= BestDistance) continue;

    const Ogre::Any& Stored =
        Candidate->getUserObjectBindings().getUserAny("EnttHandle");
    if (!Stored.has_value()) continue;

    BestDistance = Distance;
    Pick.HitEntity =
        static_cast<entt::entity>(Ogre::any_cast<uint32_t>(Stored));
    Pick.EntityDistance = Distance;
  }

  // a unit only wins if it is in front of the terrain it stands on
  if (Pick.HitEntity != entt::null && Hit.DidHit &&
      Pick.EntityDistance > Hit.Distance) {
    Pick.HitEntity = entt::null;
  }

  Event.Callback(Event.CallQueue, RayPickResult(Pick));
}

ViewPortController* RenderSystem::FindViewPortFromDevice(InputDevice* Device) {
  for (ViewPortController* VPC : ViewPorts) {
    if (VPC->IsControllerByDevice(Device)) {
      return VPC;
    }
  }
  return nullptr;
}

void RenderSystem::ChangeEntityMaterial(ChangeEntMaterialEvent Event) {
  try {
    Event.Ent->setMaterialName(Event.MatName);
  } catch (std::exception e) {
    RenderErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::MATERIAL_NOT_FOUND));
  }
}

std::vector<float> RenderSystem::GetRenderWindowDimensions() {
  float WinW = PrimaryWindow->getWidth();
  float WinH = PrimaryWindow->getHeight();
  return std::vector<float>{WinW, WinH};
}

void RenderSystem::AddOwnerShipToEnt(AddOwnerShipToEntEvent Event) {
  for (auto Ent : Event.Node->getAttachedObjects()) {
    Ogre::Entity* Entity = static_cast<Ogre::Entity*>(Ent);
    for (unsigned i = 0; i < Entity->getNumSubEntities(); ++i) {
      Entity->getSubEntity(i)->setCustomParameter(
          0, Ogre::Vector4(Event.OwnershipId, 0.0f, 0.0f, 0.0f));
    }
  }
}

void RenderSystem::UpdatePathPreview(UpdatePathPreviewEvent Event) {
  if (!PathPreviewLine) {
    PathPreviewLine = SceneManager->createManualObject("PathPreviewLine");
    SceneManager->getRootSceneNode()->attachObject(PathPreviewLine);
  }

  if (!Event.Visible || Event.Points.size() < 2) {
    PathPreviewLine->setVisible(false);
    return;
  }

  const float HalfWidth =
      GlobeInt->GetGlobeRadius() * kPathPreviewHalfWidthFraction;

  PathPreviewLine->clear();
  PathPreviewLine->begin("PATH_PREVIEW", Ogre::RenderOperation::OT_TRIANGLE_STRIP);
  AppendPathPreviewRibbon(PathPreviewLine, Event.Points,
                          GlobeInt->GetGlobeCentre(), HalfWidth);
  PathPreviewLine->end();

  // same per-section custom param unit materials read off SubEntity (see
  // AddOwnerShipToEnt) - lets PATH_PREVIEW.material discard the line on
  // every viewport except the previewing player's
  PathPreviewLine->getSection(0)->setCustomParameter(
      0, Ogre::Vector4(static_cast<float>(Event.OwnerID), 0.0f, 0.0f, 0.0f));

  PathPreviewLine->setVisible(true);
}

void RenderSystem::ChangeCameraOrbit(ChangeCameraOrbitAngleEvent Event) {
  Event.ViewportToControl->MoveCameraOrbitingPoint2DMotion(
      Event.RelativeMotion, GlobeInt->GetGlobeCentre());
}
void RenderSystem::ChangeCameraDepth(ChangeCameraOrbitDepthEvent Event) {
  Event.ViewportToControl->MoveCameraDepth(Event.WheelDelta,
                                           GlobeInt->GetGlobeCentre());
}
// singleton access to prevent 2 Ogre roots being made
RenderSystem& RenderSystem::GetInstance() {
  static RenderSystem Instance;
  return Instance;
}
