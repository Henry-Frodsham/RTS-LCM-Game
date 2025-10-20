#include "ViewPortController.h"

void ViewPortController::MoveCamera(float Pitch, float Yaw) {

}

void ViewPortController::ChangeCameraDimensions(int X, int Y) {

}

void ViewPortController::setVisibilityMask(Ogre::uint32 Mask) {

}

bool ViewPortController::ToggleAutomaticRendering(bool Val) {
	return false;
}

void ViewPortController::Update() {

}

void ViewPortController::Clear() {

}

std::vector<float> ViewPortController::GetCameraAngle() {
	return { 1.f,1.f };
}

std::vector<int> ViewPortController::GetCameraDimensions() {
	return { 1,1 };
}