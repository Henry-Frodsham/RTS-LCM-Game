// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "Player.h"

TEST_CASE("Player - initial state defaults") {
  Player P(1);
  CHECK(P.Cities == 0);
  CHECK(P.Units == 0);
  CHECK(P.AvailableCities == 1);
  CHECK(P.AvailableUnits == 1);
  CHECK(P.UnitProduction == 0);
  CHECK(P.UnitProdProg == 0.f);
}

TEST_CASE("Player - PreCityPlace returns the expected first name and "
         "increments Cities") {
  Player P(7);
  CityConstructionInfo Info = P.PreCityPlace();
  CHECK(Info.NodeName == "CityNode_7_0");
  CHECK(Info.EntName == "CityEntity_7_0");
  CHECK(P.Cities == 1);
}

TEST_CASE("Player - PreCityPlace names are unique across repeated calls") {
  Player P(2);
  CityConstructionInfo First = P.PreCityPlace();
  CityConstructionInfo Second = P.PreCityPlace();
  CHECK(First.NodeName != Second.NodeName);
  CHECK(Second.NodeName == "CityNode_2_1");
  CHECK(P.Cities == 2);
}

TEST_CASE("Player - PreUnitPlace returns the expected first name and "
         "increments Units") {
  Player P(3);
  UnitConstructionInfo Info = P.PreUnitPlace();
  CHECK(Info.NodeName == "UnitNode_3_0");
  CHECK(Info.EntName == "UnitEntity_3_0");
  CHECK(P.Units == 1);
}

TEST_CASE("Player - UpdateUnitProgressEvent increases UnitProdProg without "
         "carrying over when under 100") {
  Player P(4);
  P.PlayerQueue->Enqueue(UpdateUnitProgressEvent(60.f));
  P.Update();
  CHECK(P.UnitProdProg == 60.f);
  CHECK(P.AvailableUnits == 1);
}

TEST_CASE("Player - UnitProdProg carries into AvailableUnits once it "
         "reaches 100") {
  Player P(5);
  P.PlayerQueue->Enqueue(UpdateUnitProgressEvent(100.f));
  P.Update();
  CHECK(P.UnitProdProg == 0.f);
  CHECK(P.AvailableUnits == 2);
}

TEST_CASE("Player - a single large progress update can grant more than one "
         "unit at once") {
  Player P(6);
  P.PlayerQueue->Enqueue(UpdateUnitProgressEvent(250.f));
  P.Update();
  CHECK(P.UnitProdProg == 50.f);
  CHECK(P.AvailableUnits == 3);
}
