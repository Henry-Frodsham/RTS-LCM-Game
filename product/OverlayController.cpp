// Copyright © 2025 Henry Frodsham
#include "OverlayController.h"
#include "RenderSystem.h"

OverlayController::OverlayController() : OverlayErrorReporter() {
  OverlayMngr = Ogre::OverlayManager::getSingletonPtr();

  InitFont();
}

void OverlayController::CreateOverlay(CreateOverlayEvent Event) {
    ManagedOverlays.emplace(Event.OverlayName, OverlayMngr->create(Event.OverlayName));

    RenderSystem& Render = RenderSystem::GetInstance();


    Render.RenderQueue->Enqueue(RegisterOverlayToViewPortEvent(ManagedOverlays.at(Event.OverlayName), Render.FindViewPortFromDevice(Event.InstanceDevice)));
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

  OverlayUsed->add2D(TextPanel);
  OverlayUsed->show();
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
void OverlayController::ParentUpdate() { OverlayErrorReporter.Dispatch(); }
