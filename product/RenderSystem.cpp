//Copyright © 2025 Henry Frodsham
#include "RenderSystem.h"

void RenderSystem::RenderLoop() {

}

// creates Ogre3d root and creates primary render window
void RenderSystem::Init() {
	// create ogre root with OpenGL config file
	OgreRoot = std::make_unique<Ogre::Root>("plugins.cfg", "ogre.cfg", "ogre.log");

	// configure render system
	const Ogre::RenderSystemList& RenderSystems = OgreRoot->getAvailableRenderers();

	// error event when no Renderer is available

	// delegation to Config system for fsaa and fullscreen



}
ViewPortController* RenderSystem::CreateViewPort() {
	return NULL;
}

RenderSystem& RenderSystem::GetInstance() {
	static RenderSystem Instance;
	return Instance;
}

