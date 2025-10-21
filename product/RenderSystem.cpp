//Copyright © 2025 Henry Frodsham
#include "RenderSystem.h"

RenderSystem::RenderSystem()
	: OgreRoot(nullptr)
	, SceneManager(nullptr)
	, RenderBus(nullptr)
	, RenderQueue(nullptr)
	, PrimaryWindow(nullptr)
	, ViewPorts(NULL)
{}
RenderSystem::~RenderSystem()
{
	//shutdown, destroy root and SceneMgr
}
// main render loop, run regardless of state to maintain responsiveness
void RenderSystem::RenderLoop() {
	OgreRoot->renderOneFrame();
}

// creates Ogre3d root and creates primary render window
void RenderSystem::Init() {
	// create ogre root with OpenGL config file
	OgreRoot = std::make_unique<Ogre::Root>("plugins.cfg", "ogre.cfg", "ogre.log");

	// configure render system
	const Ogre::RenderSystemList& RenderSystems = OgreRoot->getAvailableRenderers();

	// error event when no Renderer is available

	// delegation to Config system for fsaa and fullscreen

	//for now, use the first render system (set to OpenGL in plugins.cfg)
	OgreRoot->setRenderSystem(RenderSystems[0]);

	//create scene manager
	SceneManager = OgreRoot->createSceneManager();

	//delegate ambient light and background to lighting manager (future)

}

// add a new camera to the game world, usually indicates a new split screen instance but isnt exclusive
ViewPortController* RenderSystem::CreateViewPort() {
	return NULL;
}

// singleton access to prevent 2 Ogre roots being made
RenderSystem& RenderSystem::GetInstance() {
	static RenderSystem Instance;
	return Instance;
}

