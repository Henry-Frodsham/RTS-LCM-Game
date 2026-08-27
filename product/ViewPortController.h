// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include <OGRE/OgrePrerequisites.h>

#include <vector>

#include "InputDevice.h"
#include "RayTraceEvent.h"

// view port (camera) for each split screen instance
class ViewPortController {
 private:
  Ogre::Viewport* ViewPort;
  InputDevice* ControllingDevice;

  Ogre::Real mOrbitYawRad = 0.f;
  Ogre::Real mOrbitPitchRad = 0.f;
  Ogre::Real mOrbitDistance = 0.f;
  Ogre::Real mMinOrbitDistance = 0.f;
  Ogre::Real mMaxOrbitDistance = 0.f;
 public:
  bool ToggleAutomaticRendering(bool Val);

  void setOverlaysEnabled(bool Val);

  // determines which objects will be rendered on the screen
  void setVisibilityMask(Ogre::uint32 mask);

  // change camera angle
  void MoveCamera(float Pitch, float Yaw);

  // change portion of the screen taken up by split screen
  void ChangeViewPortDimensions(float Left, float Top, float Width,
                                float Height);

  // only required when viewport is set to not automatically update in the main
  // render loop
  void Update();

  // destroys the frame buffer
  void Clear();

  // comparison with another viewport without allowing direct access to the
  // viewport
  bool Equals(Ogre::Viewport* OtherViewport);
  // motion doesnt consider a 3d coordinate because the orbit is at a fixed
  // depth
  void MoveCameraOrbitingPoint2DMotion(Ogre::Vector2f RelativeMotion,
                                       Ogre::Vector3f OrbitPoint);
  void MoveCameraDepth(float WheelDelta, Ogre::Vector3f OrbitPoint);
  void RegisterControllingDevice(InputDevice* Device);

  bool IsControllerByDevice(InputDevice* Device);

  Ogre::Ray GetWorldRayForDevice(StartRayTraceEvent Event);
  // Copy constructor - required for vector storage
  ViewPortController(const ViewPortController&) = default;
  // Move constructor
  ViewPortController(ViewPortController&&) noexcept = default;

  explicit ViewPortController(Ogre::Viewport* NewVP);
  ~ViewPortController() {}

  std::vector<float> GetCameraAngle();
  std::vector<float> GetViewPortDimensions();
  std::vector<int> GetActualDimensions();

  // the near and far planes are a fraction of whatever the camera is looking
  // at, so they are set from outside rather than fixed at construction - a
  // camera made before the globe was built has to be told again afterwards
  void SetClipDistances(Ogre::Real NearDistance, Ogre::Real FarDistance);

  void InitOrbitCamera(const Ogre::Vector3& OrbitPoint, Ogre::Real Distance);
  void SetOrbitDistanceLimits(Ogre::Real MinDistance, Ogre::Real MaxDistance);
  void ZoomOrbitingPoint(Ogre::Real Delta);
};
