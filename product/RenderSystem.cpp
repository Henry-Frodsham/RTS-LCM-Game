//Copyright © 2025 Henry Frodsham
#include "RenderSystem.h"


RenderSystem::RenderSystem()
	: OgreRoot(nullptr)
	, SceneManager(nullptr)
	, PrimaryWindow(nullptr)
	, OverlaySystem(nullptr)
	, OverlayControl(nullptr)
	, DefaultViewPort(nullptr)
	, SDLWindow(nullptr)
	, ViewPorts(NULL)
	, RenderErrorReporter(ErrorReporter())
{
	RenderBus = new EventBus();
	// only publishes to render bus so why bother specifying the bus each time
	RenderQueue = new EventQueue(RenderBus);

	DeltaTime = 1.f;

	LastFrameTime = std::chrono::high_resolution_clock::now();

	RenderConfig = new ConfigManager("VideoSettings", &RenderErrorReporter);
}
RenderSystem::~RenderSystem()
{
	if (SDLWindow) {
		SDL_DestroyWindow(SDLWindow);
	}
	SDL_Quit();

	//shutdown, destroy root and SceneMgr
}
// main render loop, run regardless of state to maintain responsiveness
void RenderSystem::RenderFrame() {
	std::chrono::steady_clock::time_point CurrentTime = std::chrono::high_resolution_clock::now();
	DeltaTime = std::chrono::duration<float>(CurrentTime - LastFrameTime).count();

	//dispatch internal queue aswell
	RenderErrorReporter.Dispatch();
	//dispatch overlay controller... etc
	UpdateExclusiveHandlers();

	if (PrimaryWindow && !PrimaryWindow->isClosed()) {
		Ogre::WindowEventUtilities::messagePump();
		RenderQueue->Dispatch();
		OgreRoot->renderOneFrame();
	}
	else {
		//window closed, shutdown app
		RenderErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::RENDER_WINDOW_CLOSED));
	}
	LastFrameTime = std::chrono::high_resolution_clock::now();
}

// creates Ogre3d root and creates primary render window
void RenderSystem::Init() {

	if (IsInit) {
		return;
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
		//sdl failed init, throw fatal
		RenderErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::SDL_FAILED_INIT));
	}

	
	#ifdef _DEBUG
		OgreRoot = new Ogre::Root("plugins_d.cfg", "", "ogre.log");
	#else
		OgreRoot = new Ogre::Root("plugins.cfg", "", "ogre.log");
	#endif

	const Ogre::RenderSystemList& RenderSystems = OgreRoot->getAvailableRenderers();

	if (RenderSystems.size() == 0) {
		RenderErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::OGRE_NO_AVAILABLE_RENDER_SYSTEM));
	}
	OgreRoot->setRenderSystem(RenderSystems[0]);

	OgreRoot->initialise(false);

	RenderWindowSettings = Ogre::RenderWindowDescription(
		RenderConfig->GetValueOrDefault<std::string>("WindowName"),
		RenderConfig->GetValueOrDefault<unsigned int>("WindowWidth"),
		RenderConfig->GetValueOrDefault<unsigned int>("WindowHeight"),
		RenderConfig->GetValueOrDefault<bool>("FullScreen")
	);

	PrimaryWindow = OgreRoot->createRenderWindow(RenderWindowSettings);

	//get the window handle to bind with SDL
	size_t WindowHandle = 0;
	PrimaryWindow->getCustomAttribute("WINDOW", &WindowHandle);

	SDLWindow = SDL_CreateWindowFrom(reinterpret_cast<void*>(WindowHandle));
	if (!SDLWindow) {
		RenderErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::SDL_FAILED_BIND));
	}

	
	SceneManager = OgreRoot->createSceneManager();
	SceneManager->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));

	OverlaySystem = new Ogre::OverlaySystem();
	SceneManager->addRenderQueueListener(OverlaySystem);

	InitBasicResourceGroups();
	
	OverlayControl = new OverlayController();

	InitRenderResponsibility();

	DefaultViewPort = CreateViewPort();
	CreateViewPort();
	IsInit = true;
}

// add a new camera to the game world, usually indicates a new split screen instance but isnt exclusive
ViewPortController* RenderSystem::CreateViewPort() {

	Ogre::SceneManager::CameraList CameraList = SceneManager->getCameras();

	Ogre::Camera* Camera = SceneManager->createCamera(std::to_string(CameraList.size()));
	Camera->setNearClipDistance(0.1f);
	Camera->setFarClipDistance(1000.0f);
	Ogre::Viewport* AddedViewPort = PrimaryWindow->addViewport(Camera, CameraList.size()); // temporary Z order

	ViewPortController* newController = new ViewPortController(AddedViewPort);

	ViewPorts.push_back(newController);

	return newController;
}

// delegates render responsibility to other classes
// think of e.g OverlayController as an arm and RenderSystem as the body
void RenderSystem::InitRenderResponsibility() {
	RenderBus->Subscribe<OverlayAddBoxEvent>(std::bind(&OverlayController::AddBox, OverlayControl, std::placeholders::_1));
	RenderBus->Subscribe<OverlayAddTextEvent>(std::bind(&OverlayController::AddText, OverlayControl, std::placeholders::_1));
	RenderBus->Subscribe<OverlayEditPanelEvent>(std::bind(&OverlayController::EditPanel, OverlayControl, std::placeholders::_1));
	RenderBus->Subscribe<OverlayEditTextEvent>(std::bind(&OverlayController::EditText, OverlayControl, std::placeholders::_1));
}

//initialise the basic resources (not game textures and mats etc)
//complex meshes and material resource groups are managed by World
//the resource groups here are basic solid colours for UI
void RenderSystem::InitBasicResourceGroups() {
	// no need to store a ptr, only used here
	Ogre::ResourceGroupManager& Rgm = Ogre::ResourceGroupManager::getSingleton();

	Rgm.createResourceGroup("Overlay");
	Rgm.createResourceGroup("Font");
	//solid colours for overlay
	std::filesystem::path PathSolDir = SOLUTION_DIR;
	Rgm.addResourceLocation(PathSolDir.append(std::string("resources\\simple\\mat\\")).generic_string(), "FileSystem", "Overlay");
	PathSolDir = SOLUTION_DIR;
	Rgm.addResourceLocation(PathSolDir.append(std::string("resources\\simple\\mesh")).generic_string(), "FileSystem", "Mesh");
	PathSolDir = SOLUTION_DIR;
	Rgm.addResourceLocation(PathSolDir.append(std::string("resources\\font\\")).generic_string(), "FileSystem", "Font");

	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Overlay");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Mesh");
	Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Font");

}

void RenderSystem::UpdateExclusiveHandlers() {
	OverlayControl->ParentUpdate();
}
float RenderSystem::GetDeltaTime() {
	return DeltaTime;
}
SDL_Window* RenderSystem::GetSDLWindow() {
	return SDLWindow;
}

Ogre::RenderWindowDescription RenderSystem::GetPrimaryWindowInformation() {
	return RenderWindowSettings;
}

Ogre::SceneNode* RenderSystem::CreateSceneNode(std::string Name) {
	return SceneManager->getRootSceneNode()->createChildSceneNode(Name);
}

Ogre::Entity* RenderSystem::CreateEntity(std::string Name, std::string MeshName) {
	return SceneManager->createEntity(Name,MeshName);
}

// singleton access to prevent 2 Ogre roots being made
RenderSystem& RenderSystem::GetInstance() {
	static RenderSystem Instance;
	return Instance;
}


