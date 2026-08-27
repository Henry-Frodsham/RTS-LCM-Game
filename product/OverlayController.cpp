// Copyright (c) 2025 Henry Frodsham
#include "OverlayController.h"

#include <string>
#include <vector>

#include "RenderSystem.h"

OverlayController::OverlayController() : OverlayErrorReporter() {
  OverlayMngr = Ogre::OverlayManager::getSingletonPtr();
  InitFont();
}

// create an ogre::overlay, link to scenemanager and viewport
// register as a managed overlay
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

// add a 2d mesh to the scene, to a preexisting overlay
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
  OverlayInfo* Info =
      new OverlayInfo(false, false, Event.OverlayToUse, Event.MaterialName);

  OverlayInfos.emplace(Event.Name, Info);
  Ogre::OverlayContainer* Panel = static_cast<Ogre::OverlayContainer*>(
      OverlayMngr->createOverlayElement("Panel", Event.Name));

  // GMM_RELATIVE coordinates are already parametrics of the viewport the
  // overlay is attached to, so they must be used verbatim. scaling them by
  // window/viewport doubled every element's x in split screen and made the
  // drawn box drift away from the caller's own coordinates
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

// add text to scene, standalone by default. attached to a transparent panel
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
    // TextArea->setMaterialName(Event.MaterialName, "Overlay");
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

// add a plain coloured box as a child of an existing panel - unlike AddBox,
// this isn't registered as a top-level 2D element of the overlay, so it
// isn't independently hover/press-checked by OverlayCursorCheck/
// OverlayPressedCheck (which return early on the first element they find
// with no registered callback). that keeps things like a slider's fill
// purely visual while still letting it be resized via EditPanel, since Ogre
// resolves element names globally regardless of nesting
void OverlayController::AddBoxToPanel(OverlayAddBoxToPanelEvent Event) {
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
        fmt::format("panel {} not found for box attachment", Event.PanelName)));
    return;
  }

  Ogre::OverlayContainer* Panel = dynamic_cast<Ogre::OverlayContainer*>(Found);
  if (!Panel) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ELEMENT_NOT_FOUND,
        fmt::format("{} exists but is not a container", Event.PanelName)));
    return;
  }

  Ogre::OverlayContainer* ChildBox = static_cast<Ogre::OverlayContainer*>(
      OverlayMngr->createOverlayElement("Panel", Event.BoxName));

  ChildBox->setMetricsMode(Ogre::GMM_RELATIVE);
  ChildBox->setPosition(Event.Position[0], Event.Position[1]);
  ChildBox->setDimensions(Event.Dimensions[0], Event.Dimensions[1]);

  try {
    ChildBox->setMaterialName(Event.MaterialName, "Overlay");
  } catch (const std::exception& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::MATERIAL_NOT_FOUND,
        fmt::format("material {} not found in resource group \"Overlay\"",
                    Event.MaterialName)));
    return;
  }

  Panel->addChild(ChildBox);

  // track it using the parent panel's overlay name from OverlayInfos, same
  // as AddTextToPanel
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
  OverlayInfos.emplace(Event.BoxName, Info);
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
  // see AddBox - GMM_RELATIVE values are viewport parametrics, never scaled
  FoundElement->setMetricsMode(Ogre::GMM_RELATIVE);
  if (Event.NewDimensions != std::vector<float>{-1.f, -1.f}) {
    FoundElement->setDimensions(Event.NewDimensions[0],
                                Event.NewDimensions[1]);
  }
  if (Event.NewPosition != std::vector<float>{-1.f, -1.f}) {
    FoundElement->setPosition(Event.NewPosition[0], Event.NewPosition[1]);
  }
  if (Event.NewMaterialName != "USE_OLD") {
    FoundElement->setMaterialName(Event.NewMaterialName, "Overlay");
  }
}

// ogre lays a caption out in the same relative space as the panel holding it,
// so a string is as wide as its glyph advances scaled by the character height
// and the viewport aspect - the same arithmetic TextAreaOverlayElement does
// when it builds its geometry
float OverlayController::MeasureCaptionWidth(
    Ogre::TextAreaOverlayElement* TextArea) {
  const Ogre::FontPtr& Font = TextArea->getFont();
  const int ViewPortWidth = OverlayMngr->getViewportWidth();

  // before the first frame is drawn there is no viewport to measure against
  if (!Font || ViewPortWidth <= 0) {
    return 0.f;
  }

  const float AspectCoefficient =
      static_cast<float>(OverlayMngr->getViewportHeight()) /
      static_cast<float>(ViewPortWidth);

  float TotalAdvance = 0.f;
  try {
    // a space falls outside the range of glyphs the font generates, so ogre
    // takes its width from a zero instead
    const float SpaceAdvance = Font->getGlyphInfo('0').advance;

    for (char Character : TextArea->getCaption()) {
      TotalAdvance +=
          (Character == ' ')
              ? SpaceAdvance
              : Font->getGlyphInfo(static_cast<unsigned char>(Character))
                    .advance;
    }
  } catch (const std::exception& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ELEMENT_NOT_FOUND,
        fmt::format("{} contains a character the overlay font cannot draw",
                    TextArea->getName())));
    return 0.f;
  }

  return TotalAdvance * TextArea->getCharHeight() * AspectCoefficient;
}

void OverlayController::FitPanelToText(OverlayFitPanelToTextEvent Event) {
  if (!OverlayMngr) {
    OverlayErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::OVERLAY_UNITIALISED));
    return;
  }

  try {
    ManagedOverlays.at(Event.OverlayToFindIn);
  } catch (const std::out_of_range& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::OVERLAY_NOT_FOUND,
        fmt::format("the request to fit {} specified overlay {} which is not "
                    "a managed overlay",
                    Event.PanelName, Event.OverlayToFindIn)));
    return;
  }

  Ogre::OverlayElement* FoundPanel = nullptr;
  Ogre::OverlayElement* FoundText = nullptr;
  try {
    FoundPanel = OverlayMngr->getOverlayElement(Event.PanelName);
    FoundText = OverlayMngr->getOverlayElement(Event.TextName);
  } catch (std::exception& e) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ELEMENT_NOT_FOUND,
        fmt::format("the request to fit {} to {} failed since {} didnt "
                    "contain both",
                    Event.PanelName, Event.TextName, Event.OverlayToFindIn)));
    return;
  }

  Ogre::TextAreaOverlayElement* TextArea =
      dynamic_cast<Ogre::TextAreaOverlayElement*>(FoundText);
  if (!TextArea) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ELEMENT_NOT_FOUND,
        fmt::format("{} exists but is not a TextArea", Event.TextName)));
    return;
  }

  const float CharHeight = TextArea->getCharHeight();
  const float PanelWidth =
      MeasureCaptionWidth(TextArea) + (Event.Padding[0] * 2.f);
  const float PanelHeight = CharHeight + (Event.Padding[1] * 2.f);

  // see AddBox - GMM_RELATIVE values are viewport parametrics, never scaled.
  // setting the metrics mode of the TextArea as well would throw away the
  // character height it was created with, so only the panel is touched
  FoundPanel->setMetricsMode(Ogre::GMM_RELATIVE);
  FoundPanel->setDimensions(PanelWidth, PanelHeight);
  FoundPanel->setPosition(Event.Centre[0] - (PanelWidth * 0.5f),
                          Event.Centre[1] - (PanelHeight * 0.5f));

  // a child is offset from its parent's corner rather than scaled by its box,
  // and a centred TextArea grows either side of its own position, so half the
  // panel width puts the caption in the middle of it
  TextArea->setAlignment(Ogre::TextAreaOverlayElement::Center);
  TextArea->setPosition(PanelWidth * 0.5f, Event.Padding[1]);
  TextArea->setDimensions(PanelWidth, PanelHeight);
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

  // the text element must always exist regardless of how it was created
  Ogre::OverlayElement* FoundText = nullptr;
  try {
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

  if (!TextArea) {
    OverlayErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ELEMENT_NOT_FOUND,
        fmt::format("{} exists but is not a TextArea", Event.NameOfExisting)));
    return;
  }

  Ogre::OverlayElement* FoundPanel = nullptr;
  bool HasOwnPanel = true;
  try {
    FoundPanel =
        OverlayMngr->getOverlayElement(Event.NameOfExisting + "_Panel");
  } catch (std::exception& e) {
    HasOwnPanel = false;
  }

  if (Event.NewDimensions != std::vector<float>{-1.f, -1.f}) {
    TextArea->setDimensions(Event.NewDimensions[0], Event.NewDimensions[1]);
    if (HasOwnPanel) {
      FoundPanel->setDimensions(Event.NewDimensions[0], Event.NewDimensions[1]);
    }
  }
  if (Event.NewPosition != std::vector<float>{-1.f, -1.f}) {
    TextArea->setPosition(Event.NewPosition[0], Event.NewPosition[1]);
    if (HasOwnPanel) {
      FoundPanel->setPosition(Event.NewPosition[0], Event.NewPosition[1]);
    }
  }
  if (Event.NewText != "USE_OLD") {
    TextArea->setCaption(Event.NewText);
  }
  if (Event.NewMaterialName != "USE_OLD") {
    TextArea->setMaterialName(Event.NewMaterialName, "Overlay");
    if (HasOwnPanel) {
      FoundPanel->setMaterialName(Event.NewMaterialName, "Overlay");
    }
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
  if (!Info->HighlightOnInteraction) {
    return;
  }
  Element->setMaterialName("BLUE", "Overlay");
}
void OverlayController::OverlayReleased(Ogre::OverlayElement* Element,
                                        OverlayInfo* Info) {
  if (!Info->HighlightOnInteraction) {
    return;
  }
  Element->setMaterialName(Info->BaseMaterial, "Overlay");
}
void OverlayController::OverlayPressed(Ogre::OverlayElement* Element,
                                       OverlayInfo* Info, float MouseX,
                                       float MouseY) {
  if (Info->HighlightOnInteraction) {
    Element->setMaterialName("PURPLE", "Overlay");
  }

  if (Info->PressCallBack != nullptr && Info->CallQueue != nullptr) {
    Info->PressCallBack(*Info->CallQueue, MouseX, MouseY);
  }
}
void OverlayController::OverlayCursorCheck(CursorMovementEvent Event) {
  // find the overlay specific to the device being moved (as to not check
  // everything for each device)
  std::string OverlayName = "UI_Overlay_" + std::to_string(Event.ThreadNumber);
  Ogre::Overlay* SpecificOverlay = OverlayMngr->getByName(OverlayName);

  if (SpecificOverlay == nullptr) {
    return;
  }

  Ogre::Overlay::OverlayContainerList ContainedElements =
      SpecificOverlay->get2DElements();

  for (Ogre::OverlayElement* Element : ContainedElements) {
    OverlayInfo* Info = nullptr;
    try {
      Info = OverlayInfos.at(Element->getName());
    } catch (std::exception e) {
      OverlayErrorReporter.EnqueueError(
          ErrorDetail::CreateError(ErrorCode::OVERLAY_MISSING_INFO));
      continue;
    }

    // a purely decorative element (a progress bar track) has no callback and
    // must not be recoloured by a cursor passing over it
    if (!Info->PressCallBack) {
      continue;
    }

    if (Element->findElementAt(Event.RelativeXY[0], Event.RelativeXY[1])) {
      Info->Hovered = true;
      // pressing the overlay should always override hovering
      if (!Info->Pressed) {
        OverlayHovered(Element, Info);
      }
    } else {
      Info->Hovered = false;
      OverlayReleased(Element, Info);
    }
  }
}
void OverlayController::OverlayPressedCheck(PressActionCommand Cmd) {
  ActionContext Cntxt = Cmd.Context;

  std::string OverlayName = "UI_Overlay_" + std::to_string(Cntxt.ThreadId);
  Ogre::Overlay* SpecificOverlay = OverlayMngr->getByName(OverlayName);

  if (SpecificOverlay == nullptr) {
    return;
  }

  Ogre::Overlay::OverlayContainerList ContainedElements =
      SpecificOverlay->get2DElements();

  for (Ogre::OverlayElement* Element : ContainedElements) {
    OverlayInfo* Info = nullptr;
    try {
      Info = OverlayInfos.at(Element->getName());
    } catch (std::exception e) {
      OverlayErrorReporter.EnqueueError(
          ErrorDetail::CreateError(ErrorCode::OVERLAY_MISSING_INFO));
      continue;
    }

    // skip this element, never the rest of the overlay - an earlier
    // callbackless element used to abort the whole check and swallow presses
    // aimed at everything behind it
    if (!Info->PressCallBack) {
      continue;
    }

    if (Element->findElementAt(Cntxt.MouseX, Cntxt.MouseY) && !Cmd.Released) {
      Info->Pressed = true;
      OverlayPressed(Element, Info, Cntxt.MouseX, Cntxt.MouseY);
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
  Info->HighlightOnInteraction = Event.Highlight;
}

void OverlayController::ParentUpdate() { OverlayErrorReporter.Dispatch(); }
