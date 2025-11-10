//Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include <OGRE/Overlay/OgreImGuiOverlay.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <OGRE/Bites/OgreWindowEventUtilities.h>
#include <SDL2/SDL.h>
#include <filesystem>
#include "ViewPortController.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "ErrorReporter.h"
#include "OverlayEvent.h"
#include "OverlayController.h"

// base rendering system in singleton pattern
// responsible for attaching models to the main game map, and for creating viewports
class RenderSystem {
private:
	RenderSystem();
	~RenderSystem();

	bool IsInit = false;

	Ogre::Root* OgreRoot;
	Ogre::SceneManager* SceneManager;

	SDL_Window* SDLWindow;

	Ogre::OverlaySystem* OverlaySystem;

	OverlayController* OverlayControl;

	Ogre::RenderWindow* PrimaryWindow;

	ErrorReporter RenderErrorReporter;
	
	ViewPortController* DefaultViewPort;
	EventBus* RenderBus;
	EventQueue* RenderQueue;

	// ogre has direct ownership of the viewport so cant use direct ownership
	std::vector<ViewPortController*> ViewPorts;

	void InitRenderResponsibility();

	void InitBasicResourceGroups();

	void UpdateExclusiveHandlers();

public:
	static RenderSystem& GetInstance();
	//RenderSystem(const RenderSystem&) = delete;
	//RenderSystem& operator=(const RenderSystem&) = delete;

	void Init();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	ViewPortController* CreateViewPort();

	void RenderFrame();

	SDL_Window* GetSDLWindow();

	

};