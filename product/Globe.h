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
  BiomeType HitBiome;
  GlobeRayHit()
      : DidHit(false),
        Distance(0.f),
        HitPoint(Ogre::Vector3::ZERO),
        SurfaceNormal(Ogre::Vector3::UNIT_Y),
        TileID(InvalidTileID),
        HitBiome(BiomeType::Ocean){}
};


struct TileSpatialIndex {
  int LonBins = 0;
  int LatBins = 0;
  std::vector<std::vector<uint32_t>> Buckets;  // size LonBins * LatBins
};

// Plain-data staging buffers for the globe's visual mesh, laid out exactly
// as BuildVisualMesh() needs them for upload. Building this only touches
// Globe/Tile state and plain containers - no Ogre resource managers - so
// it can run on a worker thread. Handing it to the GPU (BuildVisualMesh)
// still has to happen on the render thread.
struct VisualMeshBufferData {
  std::vector<float> VertexData;     // pos3 + normal3 + colour3 per vertex
  std::vector<uint32_t> IndexData;   // 3 per face
  size_t VertexCount = 0;
};


class Globe {
 public:
  Globe();

  // Builds the tile/geometry data for the globe. Pure CPU work (no Ogre
  // resource managers are touched), so this is safe to run off the render
  // thread - e.g. on a worker thread while the render thread keeps ticking.
  void Generate(unsigned int subdivisionFreq, unsigned int seed);

  // Prepares the visual mesh's vertex/index data as plain buffers. Reads
  // only Globe/Tile state, so - like Generate() - this is safe to run off
  // the render thread once Generate() has completed.
  VisualMeshBufferData BuildVisualMeshData() const;

  // Uploads BuildVisualMeshData()'s output into GPU-backed Ogre resources
  // (MeshManager, HardwareBufferManager). This touches the render system
  // and must be called on the render thread.
  Ogre::MeshPtr BuildVisualMesh(const VisualMeshBufferData& BufferData);

  // Convenience wrapper for callers that don't need the CPU/render-thread
  // split - runs both stages back to back on the calling thread.
  Ogre::MeshPtr BuildVisualMesh() { return BuildVisualMesh(BuildVisualMeshData()); }

  uint32_t GetTileCount() const { return static_cast<uint32_t>(Tiles.size()); }
  const Tile& GetTile(uint32_t id) const { return Tiles[id]; }
  Tile& GetTile(uint32_t id) { return Tiles[id]; }

  uint32_t FindTileAt(const Ogre::Vector3& directionFromCenter) const;

  // globe-local surface position/normal for a tile, elevation-displaced the
  // same way the visual mesh and CastRay's hit results are - used by
  // Pathfinder for edge costs and by GlobeInterface to place path waypoints
  Ogre::Vector3 GetTileSurfacePosition(uint32_t TileID) const {
    return GetDisplacedVertexPosition(TileID);
  }
  Ogre::Vector3 GetTileSurfaceNormal(uint32_t TileID) const {
    return ComputeApproximateNormal(TileID);
  }

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