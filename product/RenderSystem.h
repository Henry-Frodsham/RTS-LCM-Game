//Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <OGRE/Bites/OgreWindowEventUtilities.h>
#include "ViewPortController.h"
#include "EventBus.h"
#include "EventQueue.h"


// base rendering system in singleton pattern
// responsible for attaching models to the main game map, and for creating viewports
class RenderSystem {
private:
	RenderSystem();
	~RenderSystem();

	bool IsInit = false;

	Ogre::Root* OgreRoot;
	Ogre::SceneManager* SceneManager;

	Ogre::RenderWindow* PrimaryWindow;

	

	std::unique_ptr <EventBus> RenderBus;
	std::unique_ptr <EventQueue> RenderQueue;

	// ogre has direct ownership of the viewport so cant use direct ownership
	std::vector<ViewPortController*> ViewPorts;



public:
	static RenderSystem& GetInstance();
	//RenderSystem(const RenderSystem&) = delete;
	//RenderSystem& operator=(const RenderSystem&) = delete;

	void Init();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	ViewPortController* CreateViewPort();

	void RenderFrame();


};