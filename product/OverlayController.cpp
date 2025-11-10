//Copyright © 2025 Henry Frodsham
#include "OverlayController.h"

OverlayController::OverlayController() : OverlayErrorReporter()
{
	OverlayMngr = Ogre::OverlayManager::getSingletonPtr();
}

void OverlayController::AddBox(OverlayAddBoxEvent Event) {
	if (!OverlayMngr) {
		OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::OVERLAY_UNITIALISED));
		return;
	}

	Ogre::Overlay* NewOverlay = OverlayMngr->create(Event.Name);

	Ogre::OverlayContainer* Panel = static_cast<Ogre::OverlayContainer*>(
		OverlayMngr->createOverlayElement("Panel", "ColoredBox"));

	Panel->setMetricsMode(Ogre::GMM_RELATIVE);
	Panel->setPosition(Event.Position[0], Event.Position[1]);
	Panel->setDimensions(Event.Dimensions[0], Event.Dimensions[1]);
	
	try {
		Panel->setMaterialName(Event.MaterialName, "Overlay");
	}
	catch (const std::exception& e) {
		OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::MATERIAL_NOT_FOUND, fmt::format("material {} not found in resource group \"Overlay\"", Event.MaterialName)));
		//dont add an incomplete overlay container
		return;
	}
	NewOverlay->add2D(Panel);

	NewOverlay->show();
}

void OverlayController::ParentUpdate() {
	OverlayErrorReporter.Dispatch();
}