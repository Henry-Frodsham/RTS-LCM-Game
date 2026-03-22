// Copyright © 2025 Henry Frodsham
#include "Player.h"

Player::Player(Empire* Emp, int PlayerId) : PlayerEmpire(Emp), Id(PlayerId) {}

void Player::PlaceCity(ECSHelper* Factory, Ogre::Vector3 Pos) {
  Factory->FactoryQueue->Enqueue(CreateMeshWorldEntityEvent(
      "CityNode_" + std::to_string(Id) + "_" + std::to_string(Cities),
      "city.mesh",
      "CityEntity_" + std::to_string(Id) + "_" + std::to_string(Cities) ,Pos));

  Cities += 1;
}