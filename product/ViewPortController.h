#pragma once
#include <OGRE/Ogre.h>
#include <OGRE/OgrePrerequisites.h>
#include <vector>

// view port (camera) for each split screen instance
class ViewPortController {
private:
	ViewPortController* ViewPort;
	Ogre::Camera* Camera;

public:
	bool ToggleAutomaticRendering(bool Val);

	// determines which objects will be rendered on the screen
	void setVisibilityMask(Ogre::uint32 mask);

	//change camera angle
	void MoveCamera(float Pitch, float Yaw);

	//change portion of the screen taken up by split screen
	void ChangeCameraDimensions(int X, int Y);

	// only required when viewport is set to not automatically update in the main render loop
	void Update();

	// destroys the frame buffer
	void Clear();

	ViewPortController(){}
	~ViewPortController(){}

	std::vector<float> GetCameraAngle();
	std::vector<int> GetCameraDimensions();
};