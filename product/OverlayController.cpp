// Copyright © 2025 Henry Frodsham
#include "OverlayController.h"

#include "RenderSystem.h"

OverlayController::OverlayController() : OverlayErrorReporter() {
  OverlayMngr = Ogre::OverlayManager::getSingletonPtr();
  InitFont();
}

void OverlayController::CreateOverlay(CreateOverlayEvent Event) {

    if (OverlayMngr->getByName(Event.OverlayName) == nullptr) {
        ManagedOverlays.emplace(Event.OverlayName,
            OverlayMngr->create(Event.OverlayName));
    }
  
  RenderSystem& Render = RenderSystem::GetInstance();

  if (Event.InstanceDevice != nullptr) {
      Render.RenderQueue->Enqueue(RegisterOverlayToViewPortEvent(
          ManagedOverlays.at(Event.OverlayName),
          Render.FindViewPortFromDevice(Event.InstanceDevice)));
  }
}

void OverlayController::AddBox(OverlayAddBoxEvent Event) {
  if (!OverlayMngr) {
    OverlayErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::OVERLAY_UNITIALISED));
    return;
  }
  Ogre::Overlay* OverlayUsed = nullptr;
  try {
    OverlayUsed = ManagedOverlays.at(Event.OverlayToUse);
  } catch (const std::out_of_range& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::OVERLAY_NOT_FOUND,
        fmt::format("the overlay {} is not a managed overlay",
                    Event.OverlayToUse)));
    return;
  }
  OverlayInfo* Info = new OverlayInfo(false, false, Event.OverlayToUse);
  OverlayInfos.emplace(Event.Name, Info);
  Ogre::OverlayContainer* Panel = static_cast<Ogre::OverlayContainer*>(
      OverlayMngr->createOverlayElement("Panel", Event.Name));

  // relative coordinate mode, avoids manual scaling with different screen sizes
  Panel->setMetricsMode(Ogre::GMM_RELATIVE);
  Panel->setPosition(Event.Position[0], Event.Position[1]);
  Panel->setDimensions(Event.Dimensions[0], Event.Dimensions[1]);
  
  try {
    Panel->setMaterialName(Event.MaterialName, "Overlay");
  } catch (const std::exception& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::MATERIAL_NOT_FOUND,
        fmt::format("material {} not found in resource group \"Overlay\"",
                    Event.MaterialName)));
    // dont add an incomplete overlay container
    return;
  }
  OverlayUsed->add2D(Panel);

  OverlayUsed->show();
}

void OverlayController::AddText(OverlayAddTextEvent Event) {
  if (!OverlayMngr) {
    OverlayErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::OVERLAY_UNITIALISED));
    return;
  }
  Ogre::Overlay* OverlayUsed = nullptr;
  try {
    OverlayUsed = ManagedOverlays.at(Event.OverlayToUse);
  } catch (const std::out_of_range& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::OVERLAY_NOT_FOUND,
        fmt::format("the overlay {} is not a managed overlay",
                    Event.OverlayToUse)));
    return;
  }

  OverlayInfo* Info = new OverlayInfo(false, false, Event.OverlayToUse);
  OverlayInfos.emplace(Event.Name, Info);

  Ogre::TextAreaOverlayElement* TextArea =
      static_cast<Ogre::TextAreaOverlayElement*>(
          OverlayMngr->createOverlayElement("TextArea", Event.Name));

  TextArea->setMetricsMode(Ogre::GMM_RELATIVE);
  TextArea->setPosition(Event.Position[0], Event.Position[1]);
  TextArea->setDimensions(Event.Dimensions[0], Event.Dimensions[1]);
  TextArea->setCaption(Event.Text);
  TextArea->setCharHeight(0.025f);
  TextArea->setColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f, 1.0f));

  try {
    TextArea->setMaterialName(Event.MaterialName, "Overlay");
  } catch (const std::exception& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::MATERIAL_NOT_FOUND,
        fmt::format("material {} not found in resource group \"Overlay\"",
                    Event.MaterialName)));
    return;
  }

  TextArea->setFontName("OverlayFont");

  // text for overlays in ogre3d needs to be attached to a panel
  // id rather have freestanding text so ill just make the panel transparent
  Ogre::OverlayContainer* TextPanel = static_cast<Ogre::OverlayContainer*>(
      OverlayMngr->createOverlayElement("Panel", Event.Name + "_Panel"));
  TextPanel->setMetricsMode(Ogre::GMM_RELATIVE);
  TextPanel->setPosition(Event.Position[0], Event.Position[1]);
  TextPanel->setDimensions(Event.Dimensions[0], Event.Dimensions[1]);
  TextPanel->setParameter("transparent", "true");
  TextPanel->addChild(TextArea);
  OverlayInfo* PanelInfo = new OverlayInfo(false, false, Event.OverlayToUse);
  OverlayInfos.emplace(Event.Name + "_Panel", PanelInfo);
  OverlayUsed->add2D(TextPanel);
  OverlayUsed->show();
}
void OverlayController::AddTextToPanel(OverlayAddTextToPanelEvent Event) {
  if (!OverlayMngr) {
    OverlayErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::OVERLAY_UNITIALISED));
    return;
  }

  Ogre::OverlayElement* Found = nullptr;
  try {
    Found = OverlayMngr->getOverlayElement(Event.PanelName);
  } catch (std::exception& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ELEMENT_NOT_FOUND,
        fmt::format("panel {} not found for text attachment",
                    Event.PanelName)));
    return;
  }

  Ogre::OverlayContainer* Panel = dynamic_cast<Ogre::OverlayContainer*>(Found);
  if (!Panel) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ELEMENT_NOT_FOUND,
        fmt::format("{} exists but is not a container", Event.PanelName)));
    return;
  }

  Ogre::TextAreaOverlayElement* TextArea =
      static_cast<Ogre::TextAreaOverlayElement*>(
          OverlayMngr->createOverlayElement("TextArea", Event.TextName));

  TextArea->setMetricsMode(Ogre::GMM_RELATIVE);
  TextArea->setPosition(Event.Position[0], Event.Position[1]);
  TextArea->setDimensions(Event.Dimensions[0], Event.Dimensions[1]);
  TextArea->setCaption(Event.Text);
  TextArea->setCharHeight(0.025f);
  TextArea->setColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f, 1.0f));
  TextArea->setFontName("OverlayFont");

  try {
    //TextArea->setMaterialName(Event.MaterialName, "Overlay");
  } catch (const std::exception& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::MATERIAL_NOT_FOUND,
        fmt::format("material {} not found in resource group \"Overlay\"",
                    Event.MaterialName)));
    return;
  }

  Panel->addChild(TextArea);

  // track it using the parent panel's overlay name from OverlayInfos
  OverlayInfo* ParentInfo = nullptr;
  try {
    ParentInfo = OverlayInfos.at(Event.PanelName);
  } catch (std::exception& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::OVERLAY_MISSING_INFO,
        fmt::format("no OverlayInfo found for parent panel {}",
                    Event.PanelName)));
    return;
  }

  OverlayInfo* Info = new OverlayInfo(false, false, ParentInfo->OwnedByOverlay);
  OverlayInfos.emplace(Event.TextName, Info);
}
void OverlayController::EditPanel(OverlayEditPanelEvent Event) {
  if (!OverlayMngr) {
    OverlayErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::OVERLAY_UNITIALISED));
    return;
  }
  Ogre::Overlay* OverlayUsed = nullptr;
  try {
    OverlayUsed = ManagedOverlays.at(Event.OverlayToFindIn);
  } catch (const std::out_of_range& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::OVERLAY_NOT_FOUND,
        fmt::format("the request to edit {} specified overlay {} which is not "
                    "a managed overlay",
                    Event.NameOfExisting, Event.OverlayToFindIn)));
    return;
  }
  Ogre::OverlayElement* FoundElement = nullptr;
  try {
    FoundElement = OverlayMngr->getOverlayElement(Event.NameOfExisting);
  } catch (std::exception& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ELEMENT_NOT_FOUND,
        fmt::format("the request to edit {} failed since {} didnt contain it",
                    Event.NameOfExisting, Event.OverlayToFindIn)));
    return;
  }

  if (Event.NewDimensions != std::vector<float>{-1.f, -1.f}) {
    FoundElement->setDimensions(Event.NewDimensions[0], Event.NewDimensions[1]);
  }
  if (Event.NewPosition != std::vector<float>{-1.f, -1.f}) {
    FoundElement->setPosition(Event.NewPosition[0], Event.NewPosition[1]);
  }
  if (Event.NewMaterialName != "USE_OLD") {
    FoundElement->setMaterialName(Event.NewMaterialName, "Overlay");
  }
}

void OverlayController::EditText(OverlayEditTextEvent Event) {
  if (!OverlayMngr) {
    OverlayErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::OVERLAY_UNITIALISED));
    return;
  }
  Ogre::Overlay* OverlayUsed = nullptr;
  try {
    OverlayUsed = ManagedOverlays.at(Event.OverlayToFindIn);
  } catch (const std::out_of_range& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::OVERLAY_NOT_FOUND,
        fmt::format("the request to edit {} specified overlay {} which is not "
                    "a managed overlay",
                    Event.NameOfExisting, Event.OverlayToFindIn)));
    return;
  }
  Ogre::OverlayElement* FoundElement = nullptr;
  Ogre::OverlayElement* FoundText = nullptr;
  try {
    FoundElement =
        OverlayMngr->getOverlayElement(Event.NameOfExisting + "_Panel");
    FoundText = OverlayMngr->getOverlayElement(Event.NameOfExisting);
  } catch (std::exception& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ELEMENT_NOT_FOUND,
        fmt::format("the request to edit {} failed since {} didnt contain it",
                    Event.NameOfExisting, Event.OverlayToFindIn)));
    return;
  }

  Ogre::TextAreaOverlayElement* TextArea =
      dynamic_cast<Ogre::TextAreaOverlayElement*>(FoundText);

  if (Event.NewDimensions != std::vector<float>{-1.f, -1.f}) {
    FoundElement->setDimensions(Event.NewDimensions[0], Event.NewDimensions[1]);
    TextArea->setDimensions(Event.NewDimensions[0], Event.NewDimensions[1]);
  }
  if (Event.NewPosition != std::vector<float>{-1.f, -1.f}) {
    FoundElement->setPosition(Event.NewPosition[0], Event.NewPosition[1]);
    TextArea->setPosition(Event.NewPosition[0], Event.NewPosition[1]);
  }
  if (Event.NewText != "USE_OLD") {
    TextArea->setCaption(Event.NewText);
  }
  if (Event.NewMaterialName != "USE_OLD") {
    FoundElement->setMaterialName(Event.NewMaterialName, "Overlay");
    TextArea->setMaterialName(Event.NewMaterialName, "Overlay");
  }
}

void OverlayController::InitFont() {
  Ogre::FontManager* fontMgr = Ogre::FontManager::getSingletonPtr();
  Ogre::ResourcePtr customFont = fontMgr->create("OverlayFont", "Font");
  customFont->setParameter("type", "truetype");
  customFont->setParameter("source", "verdana.ttf");
  customFont->setParameter("size", "24");
  customFont->setParameter("resolution", "96");
  customFont->load();
}

void OverlayController::ChangeOverlayVisibility(
    ChangeOverlayVisibilityEvent Event) {
  // for some reason ogre is very selective about what getters throw errors and
  // which throw nullptr
  Ogre::Overlay* Overlay = OverlayMngr->getByName(Event.OverlayName);
  if (Overlay == NULL) {
    OverlayErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::OVERLAY_NOT_FOUND));
  }

  Ogre::OverlayElement* OverlayElement =
      OverlayMngr->getOverlayElement(Event.ObjectName);

  if (OverlayElement == nullptr) {
    OverlayErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::ELEMENT_NOT_FOUND));
  }

  OverlayElement->setVisible(Event.Visibility);
}

void OverlayController::OverlayHovered(Ogre::OverlayElement* Element,
                                       OverlayInfo* Info) {
  Element->setMaterialName("BLUE", "Overlay");
}
void OverlayController::OverlayReleased(Ogre::OverlayElement* Element,
                                        OverlayInfo* Info) {
  Element->setMaterialName("RED", "Overlay");
}
void OverlayController::OverlayPressed(Ogre::OverlayElement* Element,
                                       OverlayInfo* Info) {
  Element->setMaterialName("PURPLE", "Overlay");


  if (Info->PressCallBack != nullptr) {
    Info->PressCallBack(*Info->CallQueue);
  }
}
void OverlayController::OverlayCursorCheck(CursorMovementEvent Event) {
  RenderSystem& RS = RenderSystem::GetInstance();
  ViewPortController* EventVPC = RS.FindViewPortFromDevice(Event.Device);

  // find the overlay specific to the device being moved (as to not check
  // everything for each device)
  std::string OverlayName = "UI_Overlay_" + std::to_string(Event.ThreadNumber);
  Ogre::Overlay* SpecificOverlay = OverlayMngr->getByName(OverlayName);

  Ogre::Overlay::OverlayContainerList ContainedElements =
      SpecificOverlay->get2DElements();

  for (Ogre::OverlayElement* Element : ContainedElements) {
    OverlayInfo* Info = nullptr;
    try {
      Info = OverlayInfos.at(Element->getName());
    } catch (std::exception e) {
      OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::OVERLAY_MISSING_INFO));
      return;
    }

    if (Element->findElementAt(Event.RelativeXY[0], Event.RelativeXY[1])) {
      Info->Hovered = true;
      //pressing the overlay should always override hovering
      if (!Info->Pressed) {
        OverlayHovered(Element, Info);
      }
    } else {
      Info->Hovered = false;
      OverlayReleased(Element,Info);
    }
  }
}
void OverlayController::OverlayPressedCheck(PressActionCommand Cmd) {
  RenderSystem& RS = RenderSystem::GetInstance();
  ActionContext Cntxt = Cmd.Context;
  ViewPortController* EventVPC = RS.FindViewPortFromDevice(Cntxt.ActioningDevice);


  std::string OverlayName = "UI_Overlay_" + std::to_string(Cntxt.ThreadId);
  Ogre::Overlay* SpecificOverlay = OverlayMngr->getByName(OverlayName);

  Ogre::Overlay::OverlayContainerList ContainedElements =
      SpecificOverlay->get2DElements();

  for (Ogre::OverlayElement* Element : ContainedElements) {
    OverlayInfo* Info = nullptr;
    try {
      Info = OverlayInfos.at(Element->getName());
    } catch (std::exception e) {
      OverlayErrorReporter.EnqueueError(
          ErrorDetail::CreateError(ErrorCode::OVERLAY_MISSING_INFO));
      return;
    }

    
    if (Element->findElementAt(Cntxt.MouseX, Cntxt.MouseY) && !Cmd.Released) {
      Info->Pressed = true;
      OverlayPressed(Element, Info);
    } else {
      Info->Pressed = false;
      if (Info->Hovered) {
        OverlayHovered(Element, Info);
      }
    }
  }
}

void OverlayController::RegisterOnPressCallBack(
    RegisterOnPressCallBackEvent Event) {
  OverlayInfo* Info = nullptr;
  try {
    Info = OverlayInfos.at(Event.ObjectName);
  } catch (std::exception e) {
    OverlayErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::OVERLAY_MISSING_INFO));
    return;
  }

  Info->PressCallBack = Event.Callback;
  Info->CallQueue = Event.CallQueue;
}

void OverlayController::ParentUpdate() { OverlayErrorReporter.Dispatch(); }
