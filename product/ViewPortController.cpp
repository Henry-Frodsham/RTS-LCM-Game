//Copyright © 2025 Henry Frodsham
#include "ViewPortController.h"

// adjust camera viewing angle
void ViewPortController::MoveCamera(float Pitch, float Yaw) {

}

// change the size of a split screen portion
void ViewPortController::ChangeCameraDimensions(int X, int Y) {

}

// set the mask used by the camera, controls visibility of meshes with certain masks
void ViewPortController::setVisibilityMask(Ogre::uint32 Mask) {

}

// toggle whether a camera is automatically included in the renderOneFrame() function
// if untoggled then Update() needs to be called to render a viewPort
bool ViewPortController::ToggleAutomaticRendering(bool Val) {
	return false;
}

// update a specific camera when its not included in renderOneFrame()
void ViewPortController::Update() {

}

// clear the render queue of this viewPort (Ogres queue)
void ViewPortController::Clear() {

}

// get the view angles of a specific camera
std::vector<float> ViewPortController::GetCameraAngle() {
	return { 1.f,1.f };
}

// get the dimensions of a specific split screen instance
std::vector<int> ViewPortController::GetCameraDimensions() {
	return { 1,1 };
}