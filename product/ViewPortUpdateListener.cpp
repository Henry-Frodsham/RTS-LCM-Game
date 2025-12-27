// Copyright © 2025 Henry Frodsham
#include "ViewPortUpdateListener.h"

ViewPortUpdateListener::ViewPortUpdateListener()
    : VPULReporter(ErrorReporter()) {}
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
	
	//run every tick automatically by ogre so maintain state here aswell
    VPULReporter.Dispatch();
}

void ViewPortUpdateListener::AssignOverlayToViewport(ViewPortController* ViewPort, Ogre::Overlay* Overlay) {
  if (!ViewPort) {
    VPULReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ATTEMPT_ASSIGN_OVERLAY_TO_UNSET_VIEWPORT));
    return;
  }
  if (!Overlay) {
    VPULReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ATTEMPT_TO_ASSIGN_VIEWPORT_TO_UNSET_OVERLAY));
    return;
  }
  InstanceViewports.emplace(ViewPort, Overlay);
}
