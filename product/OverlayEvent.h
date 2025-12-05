#pragma once

// an event used to add a rectangle to the screen
// the event is queued then handled by the overlay manager
// from there it is attached to a scenenode and visible on the screen
// the overlays colour depends on the material used
struct OverlayAddBoxEvent {
  std::vector<float> Position;
  std::vector<float> Dimensions;

  std::string Name;
  std::string MaterialName;
  std::string OverlayToUse;

  OverlayAddBoxEvent(std::vector<float> Position, std::vector<float> Dimensions,
                     std::string Name, std::string MaterialName,
                     std::string OverlayToUse)
      : Position(Position),
        Dimensions(Dimensions),
        Name(Name),
        MaterialName(MaterialName),
        OverlayToUse(OverlayToUse) {}
};

// an event used to add text to the screen
// a font isnt specified each time, rather all text uses the same cached font
// the event is queued elsewhere then handled by overlay manager
struct OverlayAddTextEvent {
  std::vector<float> Position;
  std::vector<float> Dimensions;

  std::string Name;
  std::string Text;
  std::string MaterialName;
  std::string OverlayToUse;

  OverlayAddTextEvent(std::vector<float> Position,
                      std::vector<float> Dimensions, std::string Name,
                      std::string MaterialName, std::string OverlayToUse,
                      std::string Text)
      : Position(Position),
        Dimensions(Dimensions),
        Name(Name),
        MaterialName(MaterialName),
        OverlayToUse(OverlayToUse),
        Text(Text) {}
};

// an event used to edit a prexisting rectangle overlay
// the overlay to edit is found by the name of the overlay container inside ogre
// and the name of the overlay itself
// queued on an event queue elsewhere then handled by overlay manager
struct OverlayEditPanelEvent {
  std::string NameOfExisting;
  std::string OverlayToFindIn;

  std::vector<float> NewPosition;
  std::vector<float> NewDimensions;

  std::string NewMaterialName;

  OverlayEditPanelEvent(std::string Name, std::string OverlayToUse,
                        std::vector<float> Dimensions = {-1.f, -1.f},
                        std::vector<float> Position = {-1.f, -1.f},
                        std::string MaterialName = "USE_OLD")
      : NewPosition(Position),
        NewDimensions(Dimensions),
        NewMaterialName(MaterialName),
        OverlayToFindIn(OverlayToUse),
        NameOfExisting(Name) {}
};

// an event used to edit a prexisting text overlay
// found by getting the overlay container by name from inside ogre
// overlay containers exist inside overlays so that is specified too
// queued on an event queue elsewhere then handled by overlay manager
struct OverlayEditTextEvent {
  std::string NameOfExisting;
  std::string OverlayToFindIn;

  std::string NewText;

  std::vector<float> NewPosition;
  std::vector<float> NewDimensions;

  std::string NewMaterialName;

  OverlayEditTextEvent(std::string Name, std::string OverlayToUse,
                       std::vector<float> Position = {-1.f, -1.f},
                       std::vector<float> Dimensions = {-1.f, -1.f},
                       std::string MaterialName = "USE_OLD",
                       std::string Text = "USE_OLD")
      : NewPosition(Position),
        NewDimensions(Dimensions),
        NewMaterialName(MaterialName),
        OverlayToFindIn(OverlayToUse),
        NewText(Text),
        NameOfExisting(Name) {}
};
