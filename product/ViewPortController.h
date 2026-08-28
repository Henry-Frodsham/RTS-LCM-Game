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

  int ControllingInstanceNumber = 0;
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
  // InstanceNumber is the split screen instance (and player id) this viewport
  // belongs to, or 0 while it is still the unclaimed menu viewport. anything
  // drawing per-player screen space UI over this viewport needs it to name
  // its overlay, because ViewPortUpdateListener::preViewportUpdate reads the
  // player id off the last character of an overlay's name
  void RegisterControllingDevice(InputDevice* Device, int InstanceNumber = 0);

  int GetInstanceNumber() const { return ControllingInstanceNumber; }

  bool IsControllerByDevice(InputDevice* Device);

  Ogre::Ray GetWorldRayForDevice(StartRayTraceEvent Event);

  // the world space frustum wedge behind an on-screen rectangle, for a rubber
  // band select. the counterpart to GetWorldRayForDevice - same normalised
  // 0-1 viewport space, an area instead of a point, and the same division of
  // labour where this builds the geometry and RenderSystem runs the query
  Ogre::PlaneBoundedVolume GetWorldVolumeForRect(float Left, float Top,
                                                 float Right, float Bottom);

  // where the camera is in world space. needed by anything that has to work
  // out what this viewport can actually see, e.g. rejecting a box select hit
  // sitting on the far side of the globe
  Ogre::Vector3 GetCameraPosition() const;

  // world space to this viewport's normalised 0-1 screen space, the same
  // coordinates ActionContext::MouseX/Y and every overlay element use.
  // returns false when the point has no place on screen at all - behind the
  // camera, or outside the frustum - in which case the outputs are untouched
  bool ProjectToViewport(const Ogre::Vector3& WorldPos, float* OutX,
                         float* OutY) const;
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
