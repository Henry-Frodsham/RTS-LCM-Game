// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "Globe.h"

TEST_CASE("Globe - default center is origin and radius is one before "
         "Generate") {
  Globe G;
  CHECK(G.GetCenter() == Ogre::Vector3::ZERO);
  CHECK(G.GetRadius() == 1.f);
}

TEST_CASE("Globe - Generate produces a non-zero tile count") {
  Globe G;
  G.Generate(2, 1234);
  CHECK(G.GetTileCount() > 0);
}

TEST_CASE("Globe - every tile has either 5 or 6 neighbors") {
  Globe G;
  G.Generate(2, 1234);
  for (uint32_t i = 0; i < G.GetTileCount(); ++i) {
    const uint8_t Count = G.GetTile(i).GetNeighborCount();
    CHECK((Count == 5 || Count == 6));
  }
}

TEST_CASE("Globe - FindTileAt returns the tile owning that direction") {
  Globe G;
  G.Generate(3, 1234);
  const uint32_t SomeTile = G.GetTileCount() / 2;
  const uint32_t Found = G.FindTileAt(G.GetTile(SomeTile).GetCenter());
  CHECK(Found == SomeTile);
}

TEST_CASE("Globe - SetTransform updates center and radius") {
  Globe G;
  Ogre::Vector3 NewCenter(1.f, 2.f, 3.f);
  G.SetTransform(NewCenter, 5.f);
  CHECK(G.GetCenter() == NewCenter);
  CHECK(G.GetRadius() == 5.f);
}

TEST_CASE("Globe - BuildVisualMeshData vertex count matches tile count") {
  Globe G;
  G.Generate(2, 1234);
  VisualMeshBufferData Data = G.BuildVisualMeshData();
  CHECK(Data.VertexCount == G.GetTileCount());
  CHECK(Data.VertexData.size() == Data.VertexCount * 9);
}

TEST_CASE("Globe - BuildVisualMeshData index count is three times the face "
         "count") {
  Globe G;
  G.Generate(2, 1234);
  VisualMeshBufferData Data = G.BuildVisualMeshData();
  // an icosphere's face count is always a multiple of 20 (base faces) times
  // freq^2, so it should never be empty
  CHECK(Data.IndexData.size() > 0);
  CHECK(Data.IndexData.size() % 3 == 0);
}

TEST_CASE("Globe - CastRay hits a tile aimed at directly from outside") {
  Globe G;
  G.Generate(3, 1234);
  const uint32_t TargetTile = 0;
  const Ogre::Vector3 Dir = G.GetTile(TargetTile).GetCenter().normalisedCopy();

  // fire a ray from well outside the sphere straight back towards the centre
  Ogre::Ray Ray(Dir * 10.f, -Dir);
  GlobeRayHit Hit = G.CastRay(Ray);

  CHECK(Hit.DidHit);
  CHECK(Hit.TileID == TargetTile);
}

TEST_CASE("Globe - CastRay misses when the ray points away from the globe") {
  Globe G;
  G.Generate(2, 1234);
  Ogre::Ray Ray(Ogre::Vector3(10.f, 0.f, 0.f), Ogre::Vector3(1.f, 0.f, 0.f));
  GlobeRayHit Hit = G.CastRay(Ray);
  CHECK_FALSE(Hit.DidHit);
}
