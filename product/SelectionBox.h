// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <string>
#include <vector>

#include "InputDevice.h"
#include "RenderSystem.h"
#include "UIElement.h"

// the rubber band drawn while a box select is being dragged.
//
// four thin panels rather than one filled rectangle, because a filled one
// would sit on top of the units the player is trying to see themselves
// selecting - every overlay material in the project is opaque, so an outline
// is what an outline has to be made of.
//
// the panels are made once and only ever moved and hidden, which is the rule
// OverlayController is built around (creating overlay elements is O(n) in
// ogre, so nothing here is created per drag)
class SelectionBox : public UIElement {
 public:
  SelectionBox(std::string BoxName, InputDevice* DeviceToRespondTo,
               std::string EdgeMaterial, int ThreadId);

  void ChangeVisibility(bool Visible) override;

  // nothing to do - the panels are positioned in the overlay's own relative
  // 0-1 space, so a resized viewport rescales them without being asked
  void MaintainScaling() override {}

  // move the band to span these two corners, given in the normalised 0-1
  // viewport space ActionContext::MouseX/Y uses. the corners may be in any
  // order, so a drag up and to the left draws the same box as one down and to
  // the right
  void SetCorners(float OriginX, float OriginY, float CornerX, float CornerY);

 private:
  // one edge of the outline. Left/Top/Width/Height in overlay space
  void PlaceEdge(const std::string& EdgeName, float Left, float Top,
                 float Width, float Height) const;

  std::string OverlayName;
  // top, bottom, left, right
  std::vector<std::string> EdgeNames;

  bool Visible = false;
};
