// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include <OGRE/Overlay/OgreOverlay.h>
#include <OGRE/Overlay/OgreOverlayManager.h>
#include <OGRE/Overlay/OgrePanelOverlayElement.h>

#include <string>  // NOLINT(build/include_order)
#include <utility>  // NOLINT(build/include_order)
#include <unordered_map>  // NOLINT(build/include_order)
#include <vector>  // NOLINT(build/include_order)

#include "ErrorReporter.h"
#include "GlobeInterface.h"
#include "RenderEvent.h"
#include "ViewPortController.h"
#include "ViewPortUpdateEvent.h"

// owned by RenderSystem exclusively
//
// draws unit health bars in screen space rather than in the world. the bars
// used to be pairs of billboards parented to each unit, which had three
// problems a fraction more tuning was never going to fix:
//
//   - a billboard scales with camera distance, so on an orbit camera a bar
//     was a sub-pixel smear zoomed out and a banner zoomed in
//   - the fill was a centre-anchored BBT_POINT billboard resized by width, so
//     the bar drained inwards from both ends towards the middle instead of
//     emptying right to left
//   - every unit carried render handles for a thing that is on screen a
//     fraction of the time
//
// so a bar is a pair of overlay panels now, held in a fixed pool per viewport
// and pinned each frame to the projected position of whichever unit it has
// been lent to. the pool is the reason this is worth doing at all: bars only
// appear for units that are in combat or selected, so a handful of slots
// covers what is ever on screen, and OverlayController's rule that overlay
// elements are made once and thereafter only moved and hidden still holds.
//
// these panels are deliberately NOT registered with OverlayController. that
// class's map exists for hover and press hit testing, and a health bar must
// never be something a click can land on
class UnitIndicatorController {
 public:
  UnitIndicatorController(Ogre::OverlayManager* Manager, GlobeInterface* Globe);

  // replaces the set of units that should be showing a bar. called from the
  // render queue with a snapshot built by WorldManager::RefreshUnitIndicators
  void SyncIndicators(SyncUnitIndicatorsEvent Event);

  // projects the current snapshot into every viewport and moves the pools to
  // match. runs every frame, because the bars have to track the camera even
  // on the frames where nothing about the units themselves changed
  void Update(const std::vector<ViewPortController*>& ViewPorts);

 private:
  // one lent-out bar: a dark track with a coloured fill sharing its left
  // edge, which is all it takes for the fill to drain the right way round
  struct BarSlot {
    Ogre::OverlayElement* Track;
    Ogre::OverlayElement* Fill;
    // the fill's material is swapped by health band rather than every frame,
    // so the band last applied is remembered
    int AppliedBand;
    bool Visible;
  };

  // every slot belonging to one viewport, on its own overlay so the existing
  // per viewport show/hide in ViewPortUpdateListener::preViewportUpdate
  // applies to it for free
  struct ViewPortPool {
    Ogre::Overlay* IndicatorOverlay;
    std::vector<BarSlot> Slots;
    int InstanceNumber;
  };

  ViewPortPool* EnsurePoolFor(ViewPortController* ViewPort);
  void BuildSlot(ViewPortPool* Pool, int SlotIndex);

  void PlaceSlot(BarSlot* Slot, float CentreX, float TopY, float Ratio);
  void HideSlot(BarSlot* Slot);

  // green above two thirds, amber above a third, red below - returned as an
  // index into kFillMaterials
  static int BandForRatio(float Ratio);

  Ogre::OverlayManager* OverlayMngr;
  GlobeInterface* GlobeInt;
  ErrorReporter IndicatorErrorReporter;

  std::unordered_map<ViewPortController*, ViewPortPool> Pools;

  std::vector<UnitIndicatorEntry> Snapshot;

  // scratch, kept between frames so a frame with bars on screen doesn't have
  // to allocate. squared camera distance paired with the entry it belongs to,
  // so an overrun pool can be trimmed to the nearest units
  std::vector<std::pair<float, const UnitIndicatorEntry*>> Candidates;
};
