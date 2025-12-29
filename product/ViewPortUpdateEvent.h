// Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Overlay/OgreOverlay.h>
#include "ViewPortController.h"
struct RegisterOverlayToViewPortEvent {
	Ogre::Overlay* Overlay;
	ViewPortController* ViewPort;

	RegisterOverlayToViewPortEvent(Ogre::Overlay* Ov, ViewPortController* VP) :
		Overlay(Ov)
		,ViewPort(VP){

	}
};