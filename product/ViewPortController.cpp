// Copyright © 2025 Henry Frodsham
#include "ViewPortController.h"

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

EndRayTraceResultEvent ViewPortController::TraceRay(StartRayTraceEvent Event) {
  Ogre::Camera* RayC = ViewPort->getCamera();

  Ogre::Ray MouseRay =
      RayC->getCameraToViewportRay(Event.Point[0], Event.Point[1]);

  Event.RaySceneQuery->setRay(MouseRay);
  Ogre::RaySceneQueryResult& Result = Event.RaySceneQuery->execute();

  return (EndRayTraceResultEvent(Result, MouseRay));
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
void ViewPortController::MoveCameraOrbitingPoint2DMotion(
    Ogre::Vector2f RelativeMotion, Ogre::Vector3f OrbitPoint) {
  Ogre::Camera* Camera = ViewPort->getCamera();

  Ogre::Real Distance =
      (Camera->getParentSceneNode()->getPosition() - OrbitPoint).length();

  Camera->getParentSceneNode()->setPosition(OrbitPoint);

  Camera->getParentSceneNode()->yaw(Ogre::Radian(RelativeMotion.x));

  Camera->getParentSceneNode()->pitch(Ogre::Radian(RelativeMotion.y));

  Camera->getParentSceneNode()->translate(Ogre::Vector3(0, 0, Distance),
                                          Ogre::Node::TS_LOCAL);
}
