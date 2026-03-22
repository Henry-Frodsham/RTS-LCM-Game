// Copyright © 2025 Henry Frodsham
#pragma once
#include "Empire.h"
#include "ECSHelper.h"
#include "WorldEvent.h"

// stores and handles player information, unique to a game instance thread
class Player {
 public:
  Player(Empire* Emp, int PlayerId);
  ~Player() {}

  Empire* PlayerEmpire;

  void PlaceCity(ECSHelper* Factory, Ogre::Vector3 Pos);
 private:
  void OnCityPlace();
  void OnUnitPlace();
  void OnIncome();

  int Cities = 0;
  int Units = 0;

  int Id;
};
