// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "EventBus.h"
#include "EventQueue.h"
#include "PlayerConstructionInfo.h"
#include "PlayerEvent.h"

// stores and handles player information, unique to a game instance thread
class Player {
 public:
  explicit Player(int PlayerId);
  ~Player() {}

  // void PlaceCity(ECSHelper* Factory, Ogre::Vector3 Pos);

  CityConstructionInfo PreCityPlace();
  UnitConstructionInfo PreUnitPlace();

  void Update();

  int Cities = 0;
  int Units = 0;
  int AvailableCities = 1;
  int AvailableUnits = 1;

  int UnitProduction = 0;
  // 0-100
  float UnitProdProg = 0.f;

  EventQueue* PlayerQueue;

 private:
  EventBus* PlayerBus;
  void OnUnitPlace();
  void OnIncome();

  void UpdateUnitProgress(UpdateUnitProgressEvent Event);

  int Id;
};
