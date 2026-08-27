// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <string>
#include <vector>

#include "InputDevice.h"
#include "ViewPortController.h"
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

// add a new text overlay to an existing panel
struct OverlayAddTextToPanelEvent {
  std::string PanelName;
  std::string TextName;
  std::string Text;
  std::string MaterialName;
  std::vector<float> Position;
  std::vector<float> Dimensions;
  OverlayAddTextToPanelEvent(std::string Pn, std::string Tn, std::string txt,
                             std::string Mname, std::vector<float> Pos,
                             std::vector<float> Dim)
      : PanelName(Pn),
        TextName(Tn),
        Text(txt),
        MaterialName(Mname),
        Position(Pos),
        Dimensions(Dim) {}
};
struct CreateOverlayEvent {
  std::string OverlayName;
  // to prevent a long chain of communication requests, an overlay can be made
  // with either the viewport or device
  InputDevice* InstanceDevice;
  ViewPortController* InstanceViewPort;

  // allow cases without either for a global overlay though
  CreateOverlayEvent(std::string Overlay, InputDevice* Device = nullptr,
                     ViewPortController* Viewport = nullptr)
      : OverlayName(Overlay),
        InstanceDevice(Device),
        InstanceViewPort(Viewport) {}
};

// change visibility of an overlay, true:visible false:not visible
struct ChangeOverlayVisibilityEvent {
  std::string OverlayName;
  std::string ObjectName;
  bool Visibility;

  ChangeOverlayVisibilityEvent(std::string OName, std::string ObName, bool Vis)
      : OverlayName(OName), ObjectName(ObName), Visibility(Vis) {}
};

struct RegisterOnPressCallBackEvent {
  std::string OverlayName;
  std::string ObjectName;

  // mouse position (in the same normalised viewport space as
  // ActionContext::MouseX/Y) is bundled in alongside the queue so a callback
  // can react to where the press landed, e.g. GenericSlider using it to
  // compute a value
  std::function<void(EventQueue&, float, float)> Callback;
  EventQueue* CallQueue;

  // false leaves the element's material alone on hover/press, for controls
  // whose colour already means something (a slider track)
  bool Highlight;

  RegisterOnPressCallBackEvent(std::string Overlay, std::string Object,
                               std::function<void(EventQueue&, float, float)> Call,
                               EventQueue* Queue, bool HighlightOnPress = true)
      : OverlayName(Overlay),
        ObjectName(Object),
        Callback(Call),
        CallQueue(Queue),
        Highlight(HighlightOnPress) {}
};

// add a new box as a child of an existing panel, e.g. a slider fill rendered
// on top of its track - same convention as OverlayAddTextToPanelEvent, but
// for a plain coloured panel instead of text. position/dimensions are
// relative to the parent panel's own box (GMM_RELATIVE), not the overlay
struct OverlayAddBoxToPanelEvent {
  std::string PanelName;
  std::string BoxName;
  std::string MaterialName;
  std::vector<float> Position;
  std::vector<float> Dimensions;
  OverlayAddBoxToPanelEvent(std::string Pn, std::string Bn, std::string Mname,
                            std::vector<float> Pos, std::vector<float> Dim)
      : PanelName(Pn),
        BoxName(Bn),
        MaterialName(Mname),
        Position(Pos),
        Dimensions(Dim) {}
};

// resize a panel so the text it already holds fits inside it, with Padding
// left clear on each edge, then centre the pair on Centre. only the render
// side knows the font and the viewport it is measured against, so an element
// asks for a fit rather than guessing a box size for its own caption
struct OverlayFitPanelToTextEvent {
  std::string PanelName;
  std::string TextName;
  std::string OverlayToFindIn;

  std::vector<float> Centre;
  std::vector<float> Padding;

  OverlayFitPanelToTextEvent(std::string Panel, std::string Text,
                             std::string OverlayToUse,
                             std::vector<float> CentredOn,
                             std::vector<float> EdgePadding)
      : PanelName(Panel),
        TextName(Text),
        OverlayToFindIn(OverlayToUse),
        Centre(CentredOn),
        Padding(EdgePadding) {}
};
