// Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include <OGRE/OgrePrerequisites.h>
#include <OGRE/Overlay/OgreFontManager.h>
#include <OGRE/Overlay/OgreOverlay.h>
#include <OGRE/Overlay/OgreOverlayManager.h>
#include <OGRE/Overlay/OgrePanelOverlayElement.h>
#include <OGRE/Overlay/OgreTextAreaOverlayElement.h>
#include <fmt/core.h>

#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "ActionCommand.h"
#include "ErrorReporter.h"
#include "OverlayEvent.h"
#include "OverlayInfo.h"
#include "SharedInputEvent.h"

// owned by RenderSystem exclusively
// exclusively handles Overlay item creation and management
// overlays in ogre are seperated into "layers"
// ogre stores objects in those layers extremely inefficiently (O(n))
// thus creation is done at startup and overlays are never deleted, just hidden
class OverlayController {
 private:
  std::unordered_map<std::string, Ogre::Overlay*> ManagedOverlays;
  std::unordered_map<std::string, OverlayInfo*> OverlayInfos;
  ErrorReporter OverlayErrorReporter;

  Ogre::OverlayManager* OverlayMngr;

 public:
  // handlers for OverlayEvents on the renderQueue

  OverlayController();

  void AddBox(OverlayAddBoxEvent Event);

  void AddText(OverlayAddTextEvent Event);
  void AddTextToPanel(OverlayAddTextToPanelEvent Event);
  void EditPanel(OverlayEditPanelEvent Event);

  void EditText(OverlayEditTextEvent Event);

  void CreateOverlay(CreateOverlayEvent Event);

  void ChangeOverlayVisibility(ChangeOverlayVisibilityEvent Event);

  void OverlayCursorCheck(CursorMovementEvent Event);
  void OverlayPressedCheck(PressActionCommand Cmd);
  void RegisterOnPressCallBack(RegisterOnPressCallBackEvent Event);
  void OverlayHovered(Ogre::OverlayElement* Element, OverlayInfo* Info);
  void OverlayPressed(Ogre::OverlayElement* Element, OverlayInfo* Info);
  void OverlayReleased(Ogre::OverlayElement* Element, OverlayInfo* Info);
  void ParentUpdate();

  void InitFont();
};
