// Copyright (c) 2026 Henry Frodsham
#include "Globe.h"

#include <OgreHardwareBufferManager.h>
#include <OgreMeshManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreSubMesh.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include "FastNoiseLite.h"

namespace {

constexpr float kPhi = 1.6180339887498948482f;

struct Vec3Hasher {
  size_t operator()(const Ogre::Vector3& v) const {
    constexpr float kScale = 100000.f;
    auto qx = static_cast<int64_t>(std::llround(v.x * kScale));
    auto qy = static_cast<int64_t>(std::llround(v.y * kScale));
    auto qz = static_cast<int64_t>(std::llround(v.z * kScale));
    size_t h = std::hash<int64_t>()(qx);
    h ^= std::hash<int64_t>()(qy) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    h ^= std::hash<int64_t>()(qz) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    return h;
  }
};

struct Vec3Equal {
  bool operator()(const Ogre::Vector3& a, const Ogre::Vector3& b) const {
    constexpr float kEpsilonSq = 1e-8f;
    return a.squaredDistance(b) < kEpsilonSq;
  }
};

using VertexCache =
    std::unordered_map<Ogre::Vector3, uint32_t, Vec3Hasher, Vec3Equal>;

uint32_t GetOrAddVertex(GeodesicMesh& mesh, VertexCache& cache,
                        Ogre::Vector3 direction) {
  direction.normalise();
  auto it = cache.find(direction);
  if (it != cache.end()) return it->second;
  const uint32_t id = static_cast<uint32_t>(mesh.Vertices.size());
  mesh.Vertices.push_back(direction);
  cache.emplace(direction, id);
  return id;
}

Ogre::ColourValue BiomeToColour(BiomeType biome) {
  switch (biome) {
    case BiomeType::Ocean:
      return Ogre::ColourValue{0.10f, 0.30f, 0.55f};
    case BiomeType::Plains:
      return Ogre::ColourValue{0.45f, 0.65f, 0.25f};
    case BiomeType::Forest:
      return Ogre::ColourValue{0.15f, 0.45f, 0.15f};
    case BiomeType::Desert:
      return Ogre::ColourValue{0.80f, 0.70f, 0.40f};
    case BiomeType::Tundra:
      return Ogre::ColourValue{0.75f, 0.75f, 0.80f};
    case BiomeType::Mountain:
      return Ogre::ColourValue{0.45f, 0.40f, 0.38f};
    default:
      return Ogre::ColourValue::White;
  }
}

constexpr float kHeightScale =
    0.05f;  // max visual displacement, as a fraction of Radius

}  // namespace

Globe::Globe() : Center(Ogre::Vector3::ZERO), Radius(1.f) {}

void Globe::Generate(unsigned int subdivisionFreq, unsigned int seed) {
  VisualMesh = BuildSubdividedIcosahedron(subdivisionFreq);
  BuildTilesFromMesh(VisualMesh);
  AssignElevationAndBiome(seed);
}

GeodesicMesh Globe::BuildSubdividedIcosahedron(
    unsigned int subdivisionFreq) const {
  GeodesicMesh mesh;
  const unsigned int freq = std::max(1u, subdivisionFreq);

  // 12 base icosahedron vertices.
  std::array<Ogre::Vector3, 12> baseVerts = {{
      {-1, kPhi, 0},
      {1, kPhi, 0},
      {-1, -kPhi, 0},
      {1, -kPhi, 0},
      {0, -1, kPhi},
      {0, 1, kPhi},
      {0, -1, -kPhi},
      {0, 1, -kPhi},
      {kPhi, 0, -1},
      {kPhi, 0, 1},
      {-kPhi, 0, -1},
      {-kPhi, 0, 1},
  }};
  for (auto& v : baseVerts) v.normalise();

  // 20 base icosahedron faces (standard indexing).
  static const std::array<std::array<uint32_t, 3>, 20> baseFaces = {{
      {0, 11, 5}, {0, 5, 1},  {0, 1, 7},   {0, 7, 10}, {0, 10, 11},
      {1, 5, 9},  {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
      {3, 9, 4},  {3, 4, 2},  {3, 2, 6},   {3, 6, 8},  {3, 8, 9},
      {4, 9, 5},  {2, 4, 11}, {6, 2, 10},  {8, 6, 7},  {9, 8, 1},
  }};

  VertexCache cache;
  for (const auto& face : baseFaces) {
    const Ogre::Vector3& v0 = baseVerts[face[0]];
    const Ogre::Vector3& v1 = baseVerts[face[1]];
    const Ogre::Vector3& v2 = baseVerts[face[2]];

    // Barycentric grid over the face: point(i, j) has weights (i, j, k)
    // with i + j + k == freq.
    std::vector<std::vector<uint32_t>> grid(freq + 1);
    for (unsigned int i = 0; i <= freq; ++i) {
      grid[i].resize(freq - i + 1);
      for (unsigned int j = 0; j <= freq - i; ++j) {
        const unsigned int k = freq - i - j;
        const Ogre::Vector3 point =
            (v0 * static_cast<float>(i) + v1 * static_cast<float>(j) +
             v2 * static_cast<float>(k)) /
            static_cast<float>(freq);
        grid[i][j] = GetOrAddVertex(mesh, cache, point);
      }
    }

    // Stitch the grid into freq^2 triangles.
    for (unsigned int i = 0; i < freq; ++i) {
      for (unsigned int j = 0; j <= freq - i - 1; ++j) {
        mesh.Faces.push_back({grid[i][j], grid[i + 1][j], grid[i][j + 1]});
        if (j < freq - i - 1) {
          mesh.Faces.push_back(
              {grid[i + 1][j], grid[i + 1][j + 1], grid[i][j + 1]});
        }
      }
    }
  }

  // The barycentric walk above doesn't guarantee consistent winding, so
  // force every face to wind outward (normal pointing away from the globe
  // centre) - otherwise you'll get inside-out lighting/culling.
  for (auto& face : mesh.Faces) {
    const Ogre::Vector3& a = mesh.Vertices[face[0]];
    const Ogre::Vector3& b = mesh.Vertices[face[1]];
    const Ogre::Vector3& c = mesh.Vertices[face[2]];
    const Ogre::Vector3 centroid = (a + b + c) / 3.f;
    const Ogre::Vector3 normal = (b - a).crossProduct(c - a);
    if (normal.dotProduct(centroid) < 0.f) {
      std::swap(face[1], face[2]);
    }
  }

  return mesh;
}

void Globe::BuildTilesFromMesh(const GeodesicMesh& mesh) {
  Tiles.assign(mesh.Vertices.size(), Tile());

  std::vector<std::vector<uint32_t>> adjacency(mesh.Vertices.size());
  auto addEdge = [&adjacency](uint32_t a, uint32_t b) {
    if (std::find(adjacency[a].begin(), adjacency[a].end(), b) ==
        adjacency[a].end()) {
      adjacency[a].push_back(b);
    }
  };

  for (const auto& face : mesh.Faces) {
    addEdge(face[0], face[1]);
    addEdge(face[1], face[0]);
    addEdge(face[1], face[2]);
    addEdge(face[2], face[1]);
    addEdge(face[2], face[0]);
    addEdge(face[0], face[2]);
  }

  for (size_t i = 0; i < Tiles.size(); ++i) {
    Tiles[i].SetCenter(mesh.Vertices[i]);
    for (uint32_t neighbor : adjacency[i]) {
      Tiles[i].AddNeighbor(neighbor);
    }

    const uint8_t count = Tiles[i].GetNeighborCount();
    if (count != 5 && count != 6) {
    }
  }
}

void Globe::AssignElevationAndBiome(unsigned int seed) {
  // Placeholder tuning constants - pull these from a ContinentPreset later.
  constexpr float kNoiseFrequency = 1.6f;
  constexpr int kOctaves = 4;
  constexpr float kSeaLevel = 0.0f;

  FastNoiseLite noise;
  noise.SetSeed(static_cast<int>(seed));
  noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  noise.SetFractalType(FastNoiseLite::FractalType_FBm);
  noise.SetFractalOctaves(kOctaves);
  noise.SetFrequency(kNoiseFrequency);

  for (Tile& tile : Tiles) {
    const Ogre::Vector3& dir = tile.GetCenter();
    const float elevation = noise.GetNoise(dir.x, dir.y, dir.z);  // ~[-1, 1]
    tile.SetElevation(elevation);

    if (elevation < kSeaLevel) {
      tile.SetBiome(BiomeType::Ocean);
      continue;
    }

    const float land = (elevation - kSeaLevel) / (1.f - kSeaLevel);
    if (land < 0.15f)
      tile.SetBiome(BiomeType::Plains);
    else if (land < 0.35f)
      tile.SetBiome(BiomeType::Forest);
    else if (land < 0.60f)
      tile.SetBiome(BiomeType::Desert);
    else if (land < 0.85f)
      tile.SetBiome(BiomeType::Tundra);
    else
      tile.SetBiome(BiomeType::Mountain);
  }
}

uint32_t Globe::FindTileAt(const Ogre::Vector3& directionFromCenter) const {
  Ogre::Vector3 dir = directionFromCenter;
  dir.normalise();

  uint32_t best = InvalidTileID;
  float bestDot = -2.f;  // dot product range is [-1, 1]
  for (uint32_t i = 0; i < Tiles.size(); ++i) {
    const float d = dir.dotProduct(Tiles[i].GetCenter());
    if (d > bestDot) {
      bestDot = d;
      best = i;
    }
  }
  return best;
}

Ogre::MeshPtr Globe::BuildVisualMesh() {
  const Ogre::String meshName = "GlobeMesh";
  auto& meshMgr = Ogre::MeshManager::getSingleton();
  if (meshMgr.resourceExists(meshName)) {
    meshMgr.remove(meshName);
  }
  Ogre::MeshPtr mesh = meshMgr.createManual(
      meshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

  Ogre::SubMesh* sub = mesh->createSubMesh();
  sub->useSharedVertices = false;

  const size_t vertexCount = VisualMesh.Vertices.size();
  const size_t indexCount = VisualMesh.Faces.size() * 3;

  Ogre::VertexData* vertexData = new Ogre::VertexData();
  sub->vertexData = vertexData;
  vertexData->vertexCount = vertexCount;

  Ogre::VertexDeclaration* decl = vertexData->vertexDeclaration;
  size_t offset = 0;
  decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
  offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
  decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
  offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
  decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_DIFFUSE);

  Ogre::HardwareVertexBufferSharedPtr vBuf =
      Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
          offset + Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3),
          vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

  std::vector<float> vertexBufferData(vertexCount *
                                      9);  // pos3 + normal3 + colour3
  for (size_t i = 0; i < vertexCount; ++i) {
    const Ogre::Vector3& dir = VisualMesh.Vertices[i];
    const float elevation = Tiles[i].GetElevation();
    const Ogre::Vector3 pos =
        Center +
        dir * (Radius * (1.f + std::max(0.f, elevation) * kHeightScale));
    const Ogre::ColourValue colour = BiomeToColour(Tiles[i].GetBiome());

    float* v = &vertexBufferData[i * 9];
    v[0] = pos.x;
    v[1] = pos.y;
    v[2] = pos.z;
    v[3] = dir.x;
    v[4] = dir.y;
    v[5] = dir.z; 
    v[6] = colour.r;
    v[7] = colour.g;
    v[8] = colour.b;
  }
  vBuf->writeData(0, vBuf->getSizeInBytes(), vertexBufferData.data(), true);
  vertexData->vertexBufferBinding->setBinding(0, vBuf);

  sub->indexData->indexCount = indexCount;
  Ogre::HardwareIndexBufferSharedPtr iBuf =
      Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
          Ogre::HardwareIndexBuffer::IT_32BIT, indexCount,
          Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

  std::vector<uint32_t> indexBufferData(indexCount);
  for (size_t f = 0; f < VisualMesh.Faces.size(); ++f) {
    indexBufferData[f * 3 + 0] = VisualMesh.Faces[f][0];
    indexBufferData[f * 3 + 1] = VisualMesh.Faces[f][1];
    indexBufferData[f * 3 + 2] = VisualMesh.Faces[f][2];
  }
  iBuf->writeData(0, iBuf->getSizeInBytes(), indexBufferData.data(), true);
  sub->indexData->indexBuffer = iBuf;

  mesh->_setBounds(Ogre::AxisAlignedBox(Center - Ogre::Vector3(Radius * 1.2f),
                                        Center + Ogre::Vector3(Radius * 1.2f)));
  mesh->_setBoundingSphereRadius(Radius * 1.2f);
  mesh->load();

  return mesh;
}