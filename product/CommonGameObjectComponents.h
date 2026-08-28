// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "Player.h"

// ownership tag with player id and a pointer to the player object
// used for checking who owns x thing
struct OwnershipComponent {
  int PlayerID;
  Player* GamePlayer;
  OwnershipComponent(int Id, Player* P) : PlayerID(Id), GamePlayer(P) {}
};

// present only while a player has this entity selected, holding its
// indicators on screen for that player alone. selection is owned by
// UnitSelection over on the instance thread, but the visibility rule needs to
// read it alongside combat state, so the approved answer is mirrored into the
// registry by ECSHelper::ValidateEntitySelection and taken away again by
// ECSHelper::TryUnselectEntity
//
// PlayerID rather than a bare tag because a bar held open by a selection is
// private to the player who made it, unlike one held open by damage
struct SelectedComponent {
  int PlayerID;
  explicit SelectedComponent(int Id) : PlayerID(Id) {}
};
