//Copyright © 2025 Henry Frodsham
#include "ViewPortController.h"

// constructor
ViewPortController::ViewPortController(Ogre::Viewport* NewVP){
	ViewPort = NewVP;
}

// adjust camera viewing angle
void ViewPortController::MoveCamera(float Pitch, float Yaw) {
	ViewPort->getCamera();
}

// change the size of a split screen portion
void ViewPortController::ChangeViewPortDimensions(float Left, float Top, float Width, float Height) {
	ViewPort->setDimensions(Left,Top,Width,Height);
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
    Ogre::Camera* Camera = ViewPort->getCamera();
    const Ogre::Quaternion& Q = Camera->getDerivedOrientation();

    std::vector<float> Angles = {
        Q.getPitch().valueRadians(),
        Q.getYaw().valueRadians(),
        Q.getRoll().valueRadians()
    };
    return Angles;
}

// get the dimensions of a specific split screen instance
std::vector<int> ViewPortController::GetViewPortDimensions() {
	return std::vector<int> { ViewPort->getActualLeft(),
		ViewPort->getActualTop(),
		ViewPort->getActualWidth(),
		ViewPort->getActualHeight()
		};
}