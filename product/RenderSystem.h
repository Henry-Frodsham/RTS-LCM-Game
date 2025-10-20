//Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include <vector>
#include "ViewPortController.h"
#include "EventBus.h"
#include "EventQueue.h"

// base rendering system in singleton pattern
// responsible for attaching models to the main game map
class RenderSystem {
private:
	Ogre::Root* OgreRoot;
	Ogre::RenderWindow* PrimaryWindow;
	
	EventBus RenderBus;
	EventQueue RenderQueue;

	//direct ownership of viewports 
	std::vector<std::unique_ptr<ViewPortController>> ViewPorts;



public:
	static RenderSystem& getInstance();
	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	ViewPortController* CreateViewPort();

	void RenderLoop();

	RenderSystem() {}
	~RenderSystem() {}
};