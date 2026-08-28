// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "ECSHelper.h"
#include "InputTranslator.h"
#include "InteractionWheelCallbackCommands.h"
#include "RenderSystem.h"
#include "ShareInfoEvent.h"
#include "Tile.h"
#include "UnitSelection.h"
#include "UIEvent.h"
#include "Biome.h"
#include <entt/entt.hpp>

// the quick interaction wheel accessible through RT or right click
class InteractionWheel {
 public:
  InteractionWheel(InputTranslator* Device, int ThreadNum,
                   ECSHelper* Interactor, Player* Play);

  void UpdateAndWarmupContext();

  void OnContextActionCommand(ContextActionCommand Cmd);

  void OnPressActionCommand(PressActionCommand Cmd);

  bool HasSelection() const { return !Selection.IsEmpty(); }

  // called every frame by PlayerGeneralControl - true while the hold-to-
  // preview gesture (right click / left trigger + a selected unit) is
  // active. going false hides the preview line and forgets the last
  // hovered tile, whether that's from a release or a deselection
  void SetPreviewActive(bool Active);

  EventQueue* ForeignNotifQueue;
  ECSHelper* Factory;

  Player* GamePlayer;

 private:
  InputTranslator* DeviceState;

  EventBus* ForeignNotifBus;

  // every unit this player currently has selected. a click picks one, a box
  // drag picks as many as it encloses, and holding the add-to-selection
  // modifier makes either of those extend the set rather than replace it
  UnitSelection Selection;

  Ogre::Vector3f Position;
  Ogre::Vector3f SurfaceNormal;

  BiomeType SelectedBiome;

  bool PreviewActive = false;
  uint32_t LastPreviewedTile = InvalidTileID;

  void ShareInfoSelectedEntReceive(NotifyEntityResult Event);
  void SucessfulEntitySelection(SelectEntitySuccessEvent Event);
  void ReceiveBoxSelectResult(NotifyBoxSelectResult Event);
  void ReceiveRayResult(NotifyRayResult Event);
  // a click (LMB / right trigger) that landed on something which isn't a
  // friendly unit - an enemy entity, or empty ground. that clears the whole
  // selection, unless the add-to-selection modifier was held, which is the
  // player saying they were adding to it and simply missed
  void DeselectOnMiss(bool Additive);
  void CommitPathPreview(CommitPathPreviewEvent Event);
  void CallBackButtonA(CallBackACommand Cmd);
  void CallBackButtonB(CallBackBCommand Cmd);
  void CallBackButtonC(CallBackCCommand Cmd);
  void CallBackButtonD(CallBackDCommand Cmd);
  bool Visibility;

  int ThreadID;
};
