// Copyright (c) 2026 Henry Frodsham
#pragma once

#include <OgreMesh.h>
#include <OgreVector3.h>

#include <array>
#include <cstdint>
#include <vector>

#include "Tile.h"

// Intermediate triangulated geodesic-sphere representation. Doubles as both
// the working data during generation and the (invisible-tile-border) render
// mesh, since the hex/pentagon dual is a logic-only concept - the player
// never sees tile edges, so there's no need to build actual hexagon polygons.
struct GeodesicMesh {
  std::vector<Ogre::Vector3> Vertices;         // unit-sphere directions
  std::vector<std::array<uint32_t, 3>> Faces;  // indices into Vertices
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

 private:
  GeodesicMesh BuildSubdividedIcosahedron(unsigned int subdivisionFreq) const;
  void BuildTilesFromMesh(const GeodesicMesh& mesh);
  void AssignElevationAndBiome(unsigned int seed);

  std::vector<Tile> Tiles;
  Ogre::Vector3 Center;
  float Radius;

  GeodesicMesh VisualMesh;
};