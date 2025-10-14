#pragma once
#include <OGRE/Ogre.h>
#include <OGRE/OgrePrerequisites.h>

// view port (camera) for each split screen instance
class ViewPortController {
private:
	ViewPortController* ViewPort;
	Ogre::Camera* Camera;

public:
	bool ToggleAutomaticRendering(bool Val);

	// determines which objects will be rendered on the screen
	void setVisibilityMask(Ogre::uint32 mask);

	// only required when viewport is set to not automatically update in the main render loop
	void Update();

	// destroys the frame buffer
	void Clear();

	ViewPortController(){}
	~ViewPortController(){}
};