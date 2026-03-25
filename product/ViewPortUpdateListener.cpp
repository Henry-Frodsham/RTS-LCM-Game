// Copyright © 2025 Henry Frodsham
#include "ViewPortUpdateListener.h"

ViewPortUpdateListener::ViewPortUpdateListener()
    : VPULReporter(ErrorReporter()) {}
void ViewPortUpdateListener::preViewportUpdate(
    const Ogre::RenderTargetViewportEvent& evt) {


  int PlayerId = 0;
  // disable all the overlays, then enable the one that should be rendered
  for (auto Viewport : InstanceViewports) {
    if (Viewport.second->Equals(evt.source)) {
      Viewport.first->show();
      //the last character of an overlays name is the player id so why overcomplicate things
      char IdChar = Viewport.first->getName().back();
      PlayerId = IdChar - '0';
    } else {
      Viewport.first->hide();
    }
  }

  //activate the shader, (take a look at unit.glsl)
  //colour every other unit red on this viewport

  auto SharedParams =
      Ogre::GpuProgramManager::getSingleton().getSharedParameters(
          "ViewerParams");
  SharedParams->setNamedConstant("viewingPlayerID",
                                 static_cast<float>(PlayerId));

  // run every tick automatically by ogre so maintain state here aswell
  VPULReporter.Dispatch();
}

void ViewPortUpdateListener::AssignOverlayToViewport(
    RegisterOverlayToViewPortEvent Event) {
  if (!Event.ViewPort) {
    VPULReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ATTEMPT_ASSIGN_OVERLAY_TO_UNSET_VIEWPORT));
    return;
  }
  if (!Event.Overlay) {
    VPULReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ATTEMPT_TO_ASSIGN_VIEWPORT_TO_UNSET_OVERLAY));
    return;
  }
  InstanceViewports.emplace(Event.Overlay, Event.ViewPort);
}
