// Copyright (c) 2026 Henry Frodsham
#pragma once

#include <OgreMesh.h>
#include <OgreVector3.h>
#include <OGRE/Ogre.h>
#include <OGRE/OgreRay.h>

#include <array>
#include <cstdint>
#include <vector>

#include "Tile.h"


struct GeodesicMesh {
  std::vector<Ogre::Vector3> Vertices;         // unit-sphere directions
  std::vector<std::array<uint32_t, 3>> Faces;  // indices into Vertices
  GeodesicMesh(std::vector<Ogre::Vector3> Vert,
               std::vector<std::array<uint32_t, 3>> Fcs)
      : Vertices(Vert), Faces(Fcs) {}
  GeodesicMesh() {}
};

struct GlobeRayHit {
  bool DidHit;
  float Distance;
  Ogre::Vector3 HitPoint;         // local (globe) space
  Ogre::Vector3 SurfaceNormal;  // local (globe) space
  uint32_t TileID;
  GlobeRayHit()
      : DidHit(false),
        Distance(0.f),
        HitPoint(Ogre::Vector3::ZERO),
        SurfaceNormal(Ogre::Vector3::UNIT_Y),
        TileID(InvalidTileID) {}
};

struct TileSpatialIndex {
  int LonBins = 0;
  int LatBins = 0;
  std::vector<std::vector<uint32_t>> Buckets;  // size LonBins * LatBins
};


struct GlobeCreationConfiguration {
  unsigned int NumSubdivisions;
  unsigned int CreationSeed;
  GlobeCreationConfiguration(unsigned int NumSubDiv, unsigned int Seed)
      : NumSubdivisions(NumSubDiv), CreationSeed(Seed) {}
};

class Globe {
 public:
  Globe();

  void Generate(unsigned int subdivisionFreq, unsigned int seed);
  Ogre::MeshPtr BuildVisualMesh();

  uint32_t GetTileCount() const { return static_cast<uint32_t>(Tiles.size()); }
  const Tile& GetTile(uint32_t id) const { return Tiles[id]; }
  Tile& GetTile(uint32_t id) { return Tiles[id]; }

  uint32_t FindTileAt(const Ogre::Vector3& directionFromCenter) const;

  const Ogre::Vector3& GetCenter() const { return Center; }
  float GetRadius() const { return Radius; }

  void SetTransform(const Ogre::Vector3& center, float radius) {
    Center = center;
    Radius = radius;
  }

  GlobeRayHit CastRay(const Ogre::Ray& LocalRay) const;

 private:
  GeodesicMesh BuildSubdividedIcosahedron(unsigned int subdivisionFreq) const;
  void BuildTilesFromMesh(const GeodesicMesh& mesh);
  void AssignElevationAndBiome(unsigned int seed);
  Ogre::Vector3 GetDisplacedVertexPosition(uint32_t VertexIndex) const;
  std::vector<Tile> Tiles;
  Ogre::Vector3 Center;
  float Radius;

  TileSpatialIndex SpatialIndex;

  void BuildTileSpatialIndex();
  uint32_t FindNearestTileFast(const Ogre::Vector3& UnitDir) const;
  static std::pair<int, int> DirectionToBin(const Ogre::Vector3& UnitDir,
                                            int LonBins, int LatBins);
  Ogre::Vector3 ComputeApproximateNormal(uint32_t TileID) const;

  GeodesicMesh VisualMesh;
};