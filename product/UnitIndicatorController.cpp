// Copyright (c) 2026 Henry Frodsham
#include "UnitIndicatorController.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "RenderSystem.h"

namespace {
// bar size in normalised viewport units. these are screen space, so unlike
// the old kHealthBarWidthFraction they are NOT relative to the globe radius -
// that is the entire point of the change. a bar is the same size on screen
// whether the camera is in low orbit or backed all the way off
constexpr float kBarWidth = 0.045f;
constexpr float kBarHeight = 0.009f;
// how thick the dark surround around the fill is, per edge
constexpr float kBarBorder = 0.0018f;
// how far above the unit's projected anchor the bar sits, so it clears the
// model rather than sitting on its head
constexpr float kBarLift = 0.022f;

// how many units can show a bar at once on any one viewport. bars only appear
// for units in combat or selected, so this is generous in practice - and when
// a battle does overrun it, the units nearest the camera keep their bars (see
// Update) rather than an arbitrary handful winning
constexpr int kSlotsPerViewPort = 32;

// fill colour by health band. plain solid colour materials that already ship
// with the project, so a bar changing colour costs a material swap rather
// than a new shader
const char* const kFillMaterials[] = {"GREEN", "ORANGE", "RED"};
const char* const kTrackMaterial = "BLACK";

constexpr float kHealthyThreshold = 0.66f;
constexpr float kWoundedThreshold = 0.33f;
}  // namespace

UnitIndicatorController::UnitIndicatorController(
    Ogre::OverlayManager* Manager, GlobeInterface* Globe)
    : OverlayMngr(Manager), GlobeInt(Globe), IndicatorErrorReporter() {}

void UnitIndicatorController::SyncIndicators(SyncUnitIndicatorsEvent Event) {
  Snapshot = std::move(Event.Entries);
}

int UnitIndicatorController::BandForRatio(float Ratio) {
  if (Ratio > kHealthyThreshold) {
    return 0;
  }
  if (Ratio > kWoundedThreshold) {
    return 1;
  }
  return 2;
}

UnitIndicatorController::ViewPortPool* UnitIndicatorController::EnsurePoolFor(
    ViewPortController* ViewPort) {
  auto Existing = Pools.find(ViewPort);
  if (Existing != Pools.end()) {
    return &Existing->second;
  }

  // a viewport nobody has claimed yet is the menu viewport - it has no player
  // to draw bars for, and naming an overlay after player 0 would confuse the
  // id-off-the-last-character read in preViewportUpdate
  const int InstanceNumber = ViewPort->GetInstanceNumber();
  if (InstanceNumber <= 0) {
    return nullptr;
  }

  // the trailing number is load bearing: ViewPortUpdateListener reads the
  // player id off the last character of an overlay's name
  const std::string OverlayName =
      "UNIT_INDICATOR_OVERLAY_" + std::to_string(InstanceNumber);

  ViewPortPool Pool{};
  Pool.InstanceNumber = InstanceNumber;
  Pool.IndicatorOverlay = OverlayMngr->getByName(OverlayName);
  if (Pool.IndicatorOverlay == nullptr) {
    Pool.IndicatorOverlay = OverlayMngr->create(OverlayName);
  }

  ViewPortPool* Stored =
      &Pools.emplace(ViewPort, std::move(Pool)).first->second;

  Stored->Slots.resize(kSlotsPerViewPort);
  for (int Index = 0; Index < kSlotsPerViewPort; ++Index) {
    BuildSlot(Stored, Index);
  }

  // deliberately not shown here. the same route OverlayController::
  // CreateOverlay takes, and once the listener has this overlay it is the
  // listener that shows it on its own viewport and hides it on everybody
  // else's, every frame. showing it before that registration lands would put
  // one player's bars on every split screen viewport for a frame
  RenderSystem::GetInstance().RenderQueue->Enqueue(
      RegisterOverlayToViewPortEvent(Stored->IndicatorOverlay, ViewPort));

  return Stored;
}

void UnitIndicatorController::BuildSlot(ViewPortPool* Pool, int SlotIndex) {
  const std::string Suffix = "_" + std::to_string(Pool->InstanceNumber) + "_" +
                             std::to_string(SlotIndex);

  BarSlot& Slot = Pool->Slots[SlotIndex];
  Slot.AppliedBand = -1;
  Slot.Visible = false;

  Ogre::OverlayContainer* Track = static_cast<Ogre::OverlayContainer*>(
      OverlayMngr->createOverlayElement("Panel", "UNIT_BAR_TRACK" + Suffix));
  Track->setMetricsMode(Ogre::GMM_RELATIVE);
  Track->setDimensions(kBarWidth, kBarHeight);
  Track->setPosition(0.f, 0.f);

  Ogre::OverlayContainer* Fill = static_cast<Ogre::OverlayContainer*>(
      OverlayMngr->createOverlayElement("Panel", "UNIT_BAR_FILL" + Suffix));
  Fill->setMetricsMode(Ogre::GMM_RELATIVE);
  Fill->setDimensions(kBarWidth - (kBarBorder * 2.f),
                      kBarHeight - (kBarBorder * 2.f));
  Fill->setPosition(kBarBorder, kBarBorder);

  try {
    Track->setMaterialName(kTrackMaterial, "Overlay");
    Fill->setMaterialName(kFillMaterials[0], "Overlay");
    Slot.AppliedBand = 0;
  } catch (const std::exception& e) {
    IndicatorErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::MATERIAL_NOT_FOUND));
  }

  // the fill is a child of the track rather than a sibling, so moving the bar
  // is one setPosition on the track and the fill travels with it. the same
  // parenting AddBoxToPanel gives a slider's fill, for the same reason
  Track->addChild(Fill);
  Pool->IndicatorOverlay->add2D(Track);
  Track->hide();

  Slot.Track = Track;
  Slot.Fill = Fill;
}

void UnitIndicatorController::PlaceSlot(BarSlot* Slot, float CentreX,
                                        float TopY, float Ratio) {
  const float Clamped = std::clamp(Ratio, 0.f, 1.f);

  Slot->Track->setPosition(CentreX - (kBarWidth * 0.5f), TopY);

  // the fill is positioned relative to the track it hangs off, so its left
  // edge stays pinned at the border inset and only its width changes. that is
  // what makes it drain right to left instead of shrinking towards its own
  // centre, which is what the centre-anchored billboard got wrong
  const float FillFullWidth = kBarWidth - (kBarBorder * 2.f);
  Slot->Fill->setDimensions(FillFullWidth * Clamped,
                            kBarHeight - (kBarBorder * 2.f));

  const int Band = BandForRatio(Clamped);
  if (Band != Slot->AppliedBand) {
    try {
      Slot->Fill->setMaterialName(kFillMaterials[Band], "Overlay");
      Slot->AppliedBand = Band;
    } catch (const std::exception& e) {
      IndicatorErrorReporter.EnqueueError(
          ErrorDetail::CreateError(ErrorCode::MATERIAL_NOT_FOUND));
    }
  }

  if (!Slot->Visible) {
    Slot->Track->show();
    Slot->Visible = true;
  }
}

void UnitIndicatorController::HideSlot(BarSlot* Slot) {
  if (Slot->Visible) {
    Slot->Track->hide();
    Slot->Visible = false;
  }
}

void UnitIndicatorController::Update(
    const std::vector<ViewPortController*>& ViewPorts) {
  for (ViewPortController* ViewPort : ViewPorts) {
    if (!ViewPort) {
      continue;
    }

    ViewPortPool* Pool = EnsurePoolFor(ViewPort);
    if (!Pool) {
      continue;
    }

    const Ogre::Vector3 CameraPos = ViewPort->GetCameraPosition();

    Candidates.clear();
    for (const UnitIndicatorEntry& Entry : Snapshot) {
      // a bar held open by somebody's selection is theirs alone. one held
      // open by damage carries ExclusiveToPlayerID 0 and is shown to
      // everybody, because who is hurt and how badly is not private
      if (Entry.ExclusiveToPlayerID != 0 &&
          Entry.ExclusiveToPlayerID != Pool->InstanceNumber) {
        continue;
      }

      // a unit round the back of the planet must not leave a bar hanging over
      // the horizon, where the player can see the bar but not the unit
      if (GlobeInt &&
          GlobeInt->IsPointBeyondHorizon(CameraPos, Entry.WorldAnchor)) {
        continue;
      }

      Candidates.emplace_back((Entry.WorldAnchor - CameraPos).squaredLength(),
                              &Entry);
    }

    // more units in combat at once than there are slots is the only case
    // where this matters, and when it happens the ones the player is looking
    // closely at are the ones worth spending a slot on
    if (Candidates.size() > static_cast<std::size_t>(kSlotsPerViewPort)) {
      std::partial_sort(
          Candidates.begin(), Candidates.begin() + kSlotsPerViewPort,
          Candidates.end(),
          [](const auto& A, const auto& B) { return A.first < B.first; });
      Candidates.resize(kSlotsPerViewPort);
    }

    std::size_t Used = 0;
    for (const auto& Candidate : Candidates) {
      float ScreenX = 0.f;
      float ScreenY = 0.f;
      if (!ViewPort->ProjectToViewport(Candidate.second->WorldAnchor, &ScreenX,
                                       &ScreenY)) {
        // off screen entirely - costs no slot, so a unit fighting just past
        // the edge of the view cannot starve one that is in shot
        continue;
      }

      PlaceSlot(&Pool->Slots[Used], ScreenX, ScreenY - kBarLift,
                Candidate.second->HealthRatio);
      ++Used;
    }

    for (std::size_t Index = Used; Index < Pool->Slots.size(); ++Index) {
      HideSlot(&Pool->Slots[Index]);
    }
  }

  IndicatorErrorReporter.Dispatch();
}
