// Copyright © 2025 Henry Frodsham
#include "Player.h"
Player::Player(Empire* Emp, int PlayerId) : PlayerEmpire(Emp), Id(PlayerId) {
  PlayerBus = new EventBus();
  PlayerQueue = new EventQueue(PlayerBus);

  PlayerBus->Subscribe<UpdateUnitProgressEvent>(
      std::bind(&Player::UpdateUnitProgress, this, std::placeholders::_1));
}

/*
void Player::PlaceCity(ECSHelper* Factory, Ogre::Vector3 Pos) {
  Factory->FactoryQueue->Enqueue(CreateMeshWorldEntityEvent(
      "CityNode_" + std::to_string(Id) + "_" + std::to_string(Cities),
      "city.mesh",
      "CityEntity_" + std::to_string(Id) + "_" + std::to_string(Cities) ,Pos));

  Cities += 1;
}
*/

CityConstructionInfo Player::PreCityPlace() {
  CityConstructionInfo New = CityConstructionInfo(
      "CityNode_" + std::to_string(Id) + "_" + std::to_string(Cities),
      "CityEntity_" + std::to_string(Id) + "_" + std::to_string(Cities));
  Cities += 1;
  return New;
}

UnitConstructionInfo Player::PreUnitPlace() {
  UnitConstructionInfo New = UnitConstructionInfo(
      "UnitNode_" + std::to_string(Id) + "_" + std::to_string(Units),
      "UnitEntity_" + std::to_string(Id) + "_" + std::to_string(Units));
  Units += 1;
  return New;
}
void Player::Update() { PlayerQueue->Dispatch(); }

void Player::UpdateUnitProgress(UpdateUnitProgressEvent Event) {
  UnitProdProg += Event.AddProg;

  // while loop incase someone has so many cities that they manage to get more
  // than one unit per tick
  while (UnitProdProg >= 100.f) {
    UnitProdProg -= 100.f;
    AvailableUnits += 1;
  }
}
