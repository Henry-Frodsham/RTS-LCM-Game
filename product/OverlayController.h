//Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include <OGRE/OgrePrerequisites.h>
#include <OGRE/Overlay/OgreOverlayManager.h>
#include <OGRE/Overlay/OgrePanelOverlayElement.h>
#include <OGRE/Overlay/OgreOverlay.h>
#include <vector>
#include <fmt/core.h>
#include "ErrorReporter.h"
#include "OverlayEvent.h"

// owned by RenderSystem exclusively
// exclusively handles Overlay item creation and management
// overlays in ogre are seperated into "layers"
// ogre stores objects in those layers extremely inefficiently (O(n))
// thus creation is done at startup and overlays are never deleted, just hidden
class OverlayController {
private:
	std::vector<std::string> ManagedOverlays;

	ErrorReporter OverlayErrorReporter;

	Ogre::OverlayManager* OverlayMngr;

public:
	// handlers for OverlayEvents on the renderQueue
	OverlayController();

	void AddBox(OverlayAddBoxEvent Event);

	void EditPanel();

	void ParentUpdate();
};