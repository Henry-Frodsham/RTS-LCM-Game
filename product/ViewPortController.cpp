// Copyright (c) 2025 Henry Frodsham
#include "ViewPortController.h"

#include <vector>

// constructor
ViewPortController::ViewPortController(Ogre::Viewport* NewVP) {
  ViewPort = NewVP;
  ControllingDevice = nullptr;
}

void ViewPortController::setOverlaysEnabled(bool Val) {
  ViewPort->setOverlaysEnabled(Val);
}

void ViewPortController::RegisterControllingDevice(InputDevice* Device) {
  ControllingDevice = Device;
}

bool ViewPortController::IsControllerByDevice(InputDevice* Device) {
  return ControllingDevice == Device;
}

bool ViewPortController::Equals(Ogre::Viewport* OtherViewport) {
  return OtherViewport == ViewPort;
}

// adjust camera viewing angle
void ViewPortController::MoveCamera(float Pitch, float Yaw) {
  ViewPort->getCamera();
}

// change the size of a split screen portion
void ViewPortController::ChangeViewPortDimensions(float Left, float Top,
                                                  float Width, float Height) {
  ViewPort->setDimensions(Left, Top, Width, Height);
}

// set the mask used by the camera, controls visibility of meshes with certain
// masks
void ViewPortController::setVisibilityMask(Ogre::uint32 Mask) {}

// toggle whether a camera is automatically included in the renderOneFrame()
// function if untoggled then Update() needs to be called to render a viewPort
bool ViewPortController::ToggleAutomaticRendering(bool Val) { return false; }

// update a specific camera when its not included in renderOneFrame()
void ViewPortController::Update() {}

// clear the render queue of this viewPort (Ogres queue)
void ViewPortController::Clear() {}

// get the view angles of a specific camera
std::vector<float> ViewPortController::GetCameraAngle() {
  Ogre::Camera* Camera = ViewPort->getCamera();
  const Ogre::Quaternion& Q = Camera->getDerivedOrientation();

  std::vector<float> Angles = {Q.getPitch().valueRadians(),
                               Q.getYaw().valueRadians(),
                               Q.getRoll().valueRadians()};
  return Angles;
}

// builds the world-space ray from the viewport's camera - no scene query
// involved, the caller (RenderSystem) now tests this ray against the globe
// mesh directly via GlobeInterface::CastRayFromWorld
Ogre::Ray ViewPortController::GetWorldRayForDevice(StartRayTraceEvent Event) {
  Ogre::Camera* RayC = ViewPort->getCamera();
  return RayC->getCameraToViewportRay(Event.Point[0], Event.Point[1]);
}

// get the dimensions of a specific split screen instance
std::vector<float> ViewPortController::GetViewPortDimensions() {
  return std::vector<float>{ViewPort->getLeft(), ViewPort->getTop(),
                            ViewPort->getWidth(), ViewPort->getHeight()};
}
std::vector<int> ViewPortController::GetActualDimensions() {
  return std::vector<int>{ViewPort->getActualWidth(),
                            ViewPort->getActualHeight()};
}

void ViewPortController::SetOrbitDistanceLimits(Ogre::Real MinDistance,
                                                Ogre::Real MaxDistance) {
  mMinOrbitDistance = MinDistance;
  mMaxOrbitDistance = MaxDistance;
}

void ViewPortController::InitOrbitCamera(const Ogre::Vector3& OrbitPoint,
                                         Ogre::Real Distance) {
  mOrbitYawRad = 0.f;
  mOrbitPitchRad = 0.f;
  mOrbitDistance =
      Ogre::Math::Clamp(Distance, mMinOrbitDistance, mMaxOrbitDistance);

  Ogre::SceneNode* CamNode = ViewPort->getCamera()->getParentSceneNode();
  CamNode->setOrientation(Ogre::Quaternion::IDENTITY);
  CamNode->setPosition(OrbitPoint + Ogre::Vector3(0, 0, mOrbitDistance));
}

void ViewPortController::ZoomOrbitingPoint(Ogre::Real Delta) {
  mOrbitDistance = Ogre::Math::Clamp(mOrbitDistance + Delta, mMinOrbitDistance,
                                     mMaxOrbitDistance);
}

void ViewPortController::MoveCameraOrbitingPoint2DMotion(
    Ogre::Vector2f RelativeMotion, Ogre::Vector3f OrbitPoint) {
  constexpr Ogre::Real kPitchLimitRad = 1.53589f;  // ~88 degrees

  mOrbitYawRad -= RelativeMotion.x;
  mOrbitPitchRad -= RelativeMotion.y;
  mOrbitPitchRad =
      Ogre::Math::Clamp(mOrbitPitchRad, -kPitchLimitRad, kPitchLimitRad);

  Ogre::Camera* Camera = ViewPort->getCamera();
  Ogre::SceneNode* CamNode = Camera->getParentSceneNode();

  const Ogre::Quaternion YawRot(Ogre::Radian(mOrbitYawRad),
                                Ogre::Vector3::UNIT_Y);
  const Ogre::Quaternion PitchRot(Ogre::Radian(mOrbitPitchRad),
                                  Ogre::Vector3::UNIT_X);
  const Ogre::Quaternion Orientation = YawRot * PitchRot;

  CamNode->setOrientation(Orientation);
  CamNode->setPosition(OrbitPoint +
                       Orientation * Ogre::Vector3(0, 0, mOrbitDistance));
}
void ViewPortController::MoveCameraDepth(int ScrollWheelY,
                                         Ogre::Vector3f OrbitPoint) {
  constexpr Ogre::Real kTempZoomSensitivity =
      0.1f;  // TODO: expose as configurable setting

  Ogre::Real Delta =
      -static_cast<Ogre::Real>(ScrollWheelY) * kTempZoomSensitivity;
  ZoomOrbitingPoint(Delta);

  Ogre::Camera* Camera = ViewPort->getCamera();
  Ogre::SceneNode* CamNode = Camera->getParentSceneNode();

  const Ogre::Quaternion Orientation = CamNode->getOrientation();
  CamNode->setPosition(OrbitPoint +
                       Orientation * Ogre::Vector3(0, 0, mOrbitDistance));
}