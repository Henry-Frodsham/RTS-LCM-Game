//Copyright © 2025 Henry Frodsham
#pragma once
#include <string>
#include <OGRE/Ogre.h>

struct CreateEntityEvent {
	std::string EntityName;
	std::string MeshName;

	std::function<void(Ogre::SceneNode*, Ogre::Entity*)> StoreEntityCallBack;
};

struct TickEvent {
	float DeltaTime;
};
