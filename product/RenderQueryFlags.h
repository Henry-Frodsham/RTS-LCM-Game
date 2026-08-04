// Copyright (c) 2026 Henry Frodsham
#include <OGRE/Ogre.h>

#pragma once
namespace RenderQueryFlags {
constexpr Ogre::uint32 kGlobe = 1u << 0;
constexpr Ogre::uint32 kSelectableUnit = 1u << 1;
constexpr Ogre::uint32 kDecoration = 1u << 2;
}  // namespace RenderQueryFlags
