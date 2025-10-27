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

	if (IsInit) {
		return;
	}

	
	

	
	#ifdef _DEBUG
		OgreRoot = new Ogre::Root("plugins_d.cfg", "", "ogre.log");
	#else
		OgreRoot = new Ogre::Root("plugins.cfg", "", "ogre.log");
	#endif

	const Ogre::RenderSystemList& RenderSystems = OgreRoot->getAvailableRenderers();

	OgreRoot->setRenderSystem(RenderSystems[0]);

	PrimaryWindow = OgreRoot->initialise(true, "RTS LCM GAME");

	SceneManager = OgreRoot->createSceneManager();

	IsInit = true;
}

// add a new camera to the game world, usually indicates a new split screen instance but isnt exclusive
ViewPortController* RenderSystem::CreateViewPort() {

	Ogre::SceneManager::CameraList CameraList = SceneManager->getCameras();

	Ogre::Camera* Camera = SceneManager->createCamera(std::to_string(CameraList.size()));

	Ogre::Viewport* AddedViewPort = PrimaryWindow->addViewport(Camera, CameraList.size());

	ViewPortController* newController = new ViewPortController(AddedViewPort);

	ViewPorts.push_back(newController);

	return newController;
}

// singleton access to prevent 2 Ogre roots being made
RenderSystem& RenderSystem::GetInstance() {
	static RenderSystem Instance;
	return Instance;
}

