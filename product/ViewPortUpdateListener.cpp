// Copyright © 2025 Henry Frodsham
#include "ViewPortUpdateListener.h"

void ViewPortUpdateListener::preViewportUpdate(const Ogre::RenderTargetViewportEvent &evt) {
	//disable all the overlays, then enable the one that should be rendered
	for (auto Viewport : InstanceViewports) {
		if (Viewport.second->Equals(evt.source)) {
			Viewport.second->setOverlaysEnabled(true);
		}
		else {
			Viewport.second->setOverlaysEnabled(false);
		}
	}
	
}

void ViewPortUpdateListener::AssignOverlayToViewport(ViewPortController* vp, Ogre::Overlay* Overlay) {
	InstanceViewports.emplace(vp, Overlay);
}
