// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "BaseState.h"
#include "ConfigEvent.h"
#include "ConfigManager.h"
#include "ErrorReporter.h"
#include "GenericButton.h"
#include "GenericSlider.h"
#include "InputDevice.h"

// shared behaviour for any options page. it owns a ConfigManager per config
// file the page touches, lays its controls out for itself, and already carries
// the back and apply buttons every page needs, so a page only has to say which
// settings it shows
//
// nothing a control is dragged or clicked to reaches a file straight away - it
// is staged, and only apply writes it and announces the reload. leaving by the
// back button therefore throws the edits away, which is what having an apply
// button implies, and the controls go back to the stored values on the way in
//
// a page describes itself in OnBindSettings, one call per setting:
//
//   void MyOptions::OnBindSettings() {
//     BindSlider<float>("cursor_sensitivity", "Sensitivity", "InputSettings",
//                       "CursorSensitivity", 100.f, 3000.f);
//     BindSlider<unsigned int>("window_width", "Width", "VideoSettings",
//                              "WindowWidth", 640.f, 3840.f);
//   }
//
// where "InputSettings" is a config file's base name and "CursorSensitivity" a
// key inside it - nothing else. the control is laid out for you, starts on the
// value already in the file, shows the value it is currently on, stages every
// change, and no page has to know where that file is, when it is written, or
// who reads it
//
// the template argument is the type the key is stored as, so an integer
// setting is written back to json as an integer rather than as the float the
// track happens to work in
//
// PageName is what the page's elements are named after. two pages belonging to
// the same instance are drawn into the same overlay, and an overlay element's
// name is unique across it, so "back" on one page and "back" on the next have
// to be told apart by something - the page they are on
class BaseOptionState : public BaseState {
 public:
  BaseOptionState(EventQueue* CallBacksQueue, AppState StateOwned,
                  InputDevice* PageDevice, int Owner, std::string PageName,
                  std::string ConfigInstanceName = "");

 protected:
  // build the page's settings here, one Bind call each
  virtual void OnBindSettings() {}

  // where the back button leads. a page opened from the menu goes back to it,
  // and a page opened as a tab of another page goes back to that page instead
  virtual AppState GetBackState() const { return AppState::MENU; }

  // the page was entered / left, after the controls have been refreshed and
  // after any staged edits have been dropped respectively
  virtual void OnPageEnter() {}
  virtual void OnPageExit() {}

  // a labelled track over a numeric key, reading Minimum to Maximum
  template <typename DataType>
  GenericSlider* BindSlider(std::string ElementName, std::string LabelText,
                            const std::string& ConfigBaseName, std::string Key,
                            float Minimum, float Maximum) {
    return BindSlider<DataType>(std::move(ElementName), std::move(LabelText),
                                Config(ConfigBaseName), std::move(Key),
                                Minimum, Maximum);
  }

  template <typename DataType>
  GenericSlider* BindSlider(std::string ElementName, std::string LabelText,
                            ConfigManager* Target, std::string Key,
                            float Minimum, float Maximum) {
    // the drag has to be able to relabel the slider it belongs to, and the
    // slider does not exist until it has been given that callback, so the
    // callback is handed a holder that is filled in immediately afterwards
    std::shared_ptr<GenericSlider*> Handle =
        std::make_shared<GenericSlider*>(nullptr);

    const std::string Label = LabelText;
    const float Stored = StoredAsFloat<DataType>(Target, Key);

    GenericSlider* Created = CreateElement<GenericSlider>(
        PageElementName(ElementName), ValueLabel<DataType>(Label, Stored),
        TakeNextSlot(), SliderSize(), Minimum, Maximum, Device,
        [Handle, Target, Key, Label](EventQueue& Queue, float Value) {
          Target->StageValue<DataType>(Key, Quantise<DataType>(Value));
          (*Handle)->SetLabelText(ValueLabel<DataType>(Label, Value));
        },
        AppQueue, GetOwner());

    *Handle = Created;
    Created->SetValue(Stored);

    AddRefresher([Created, Target, Key, Label]() {
      const float Current = StoredAsFloat<DataType>(Target, Key);
      Created->SetValue(Current);
      Created->SetLabelText(ValueLabel<DataType>(Label, Current));
    });

    PageReporter.Dispatch();
    return Created;
  }

  // one component of a key held as an array of numbers - a position or a pair
  // of dimensions. the whole array is staged every time, since that is what
  // the key is, but only the named component is touched
  GenericSlider* BindComponentSlider(std::string ElementName,
                                     std::string LabelText,
                                     const std::string& ConfigBaseName,
                                     std::string Key, size_t Component,
                                     float Minimum, float Maximum);
  GenericSlider* BindComponentSlider(std::string ElementName,
                                     std::string LabelText,
                                     ConfigManager* Target, std::string Key,
                                     size_t Component, float Minimum,
                                     float Maximum);

  // a button over a boolean key, captioned with the value it currently holds.
  // a stand in until there is a tickbox element - swapping the element out
  // here changes every bool setting at once, and no page has to be touched
  GenericButton* BindToggle(std::string ElementName, std::string LabelText,
                            const std::string& ConfigBaseName,
                            std::string Key);
  GenericButton* BindToggle(std::string ElementName, std::string LabelText,
                            ConfigManager* Target, std::string Key);

  // the manager for one config file, created the first time it is asked for
  // and kept for the life of the page. the page's own instance name is used
  // unless one is named, which is how a page that is otherwise application
  // wide reaches a single player's file
  ConfigManager* Config(const std::string& ConfigBaseName);
  ConfigManager* Config(const std::string& ConfigBaseName,
                        const std::string& ConfigInstanceName);

  // stage a value for anything the Bind helpers do not cover - a control this
  // page builds itself, or a key that is not a number or a flag
  template <typename DataType>
  void Stage(const std::string& ConfigBaseName, const std::string& Key,
             DataType NewValue) {
    Config(ConfigBaseName)->StageValue<DataType>(Key, NewValue);
  }

  // called whenever the page is entered, to put a control back on the stored
  // value. a hand built control registers its own so it is refreshed with the
  // rest rather than being left showing a discarded edit
  void AddRefresher(std::function<void()> Refresher);

  // the position the next bound control will be given, and the one after it.
  // a page laying something out by hand takes a slot so it does not land on
  // top of a setting bound after it
  std::vector<float> TakeNextSlot();

  static std::vector<float> SliderSize();

  // the name an element built by this page should be given. a page building a
  // control by hand names it through here for the same reason the Bind helpers
  // do - so it cannot collide with an element of the same purpose on another
  // page in the same overlay
  std::string PageElementName(const std::string& ElementSuffix) const;

  InputDevice* GetDevice() const;

 private:
  void OnInit() final;
  void OnEnter() final;
  void OnExit() final;

  // write every config the page has edited, reload it, and tell whoever was
  // reading it to do the same
  void ApplyStagedSettings();

  void RefreshControls();

  // the track works in floats whatever the key is stored as, so every read
  // out of a config comes back through here
  template <typename DataType>
  static float StoredAsFloat(ConfigManager* Target, const std::string& Key) {
    return static_cast<float>(Target->GetValueOrDefault<DataType>(Key));
  }

  // a track is continuous and an integer setting is not, so a value on its way
  // back into the file is rounded to the nearest whole one rather than being
  // truncated towards zero by the cast
  template <typename DataType>
  static DataType Quantise(float Value) {
    if constexpr (std::is_integral_v<DataType>) {
      return static_cast<DataType>(std::llround(Value));
    } else {
      return static_cast<DataType>(Value);
    }
  }

  // the label is the only thing that says what a track is currently set to, so
  // it carries the value as well as the name, written the way the key is
  // stored rather than always as a float
  template <typename DataType>
  static std::string ValueLabel(const std::string& LabelText, float Value) {
    if constexpr (std::is_integral_v<DataType>) {
      return LabelText + ": " +
             std::to_string(static_cast<int64_t>(std::llround(Value)));
    } else {
      return LabelText + ": " + fmt::format("{:.2f}", Value);
    }
  }

  static std::string ToggleCaption(const std::string& LabelText, bool Value);

  InputDevice* Device;
  std::string Page;
  std::string InstanceName;

  // a page's configs report their own failures, and are not part of any
  // instance that already has a reporter, so the page keeps one
  ErrorReporter PageReporter;

  // keyed by base name and instance name together, since a page can hold both
  // the application's copy of a file and a single player's
  std::unordered_map<std::string, std::unique_ptr<ConfigManager>> Configs;

  std::vector<std::function<void()>> Refreshers;

  GenericButton* BackButton;
  GenericButton* ApplyButton;

  int BoundControls;
};
