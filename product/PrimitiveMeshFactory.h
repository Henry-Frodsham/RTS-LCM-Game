// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <string>

// procedurally generated placeholder geometry.
//
// the game used to load authored .mesh files off disk for its world objects.
// they carried no detail worth keeping and every new object type meant a new
// asset, which is the wrong trade while the game is still being prototyped -
// so the shapes are built in code instead, out of nothing but boxes, prisms
// and cones:
//
//   kUnit - a vertical tube with a cone on its side, pointing along local +Z
//   kBoat - a hull with a rectangular body and a pointed bow, plus a cabin
//   kCity - a single tall rectangular block
//
// every dimension is a fraction of the globe radius, the same convention the
// facing arrow (kFacingArrowLengthFraction) and health bars
// (kHealthBarWidthFraction) already use, so the shapes stay proportionate
// whatever PlanetRadius the globe was generated at
namespace PrimitiveMesh {

// mesh resource names. these are ordinary Ogre mesh names, not file names -
// EnsureMesh registers them with the MeshManager so Ogre::SceneManager::
// createEntity resolves them without ever touching the filesystem
extern const char kUnit[];
extern const char kBoat[];
extern const char kCity[];

// material the generated shapes are built with. shares the ownership-aware
// unit shaders, so the per-viewport "not yours" tint still applies
extern const char kMaterial[];
extern const char kSelectedMaterial[];

bool IsPrimitiveName(const std::string& Name);

// builds Name into the MeshManager if it is a primitive name and is not
// already there. safe to call on every entity creation - a name that isn't
// ours, or a mesh that already exists, is a no-op
void EnsureMesh(Ogre::SceneManager* SceneMngr, const std::string& Name,
                float GlobeRadius);

}  // namespace PrimitiveMesh
