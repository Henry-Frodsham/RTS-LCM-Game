// Copyright (c) 2026 Henry Frodsham
#include "BaseOptionState.h"

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
// controls fill a column top to bottom and then start the next one, so a page
// with a lot of settings on it grows sideways instead of off the bottom of the
// screen. a slider draws its label above its track, so the row spacing has to
// clear both
constexpr float FirstColumnX = 0.05f;
constexpr float ColumnSpacing = 0.31f;
constexpr int RowsPerColumn = 6;

constexpr float FirstRowY = 0.22f;
constexpr float RowSpacing = 0.115f;

constexpr float SliderWidth = 0.26f;
constexpr float SliderHeight = 0.03f;

// back sits where it does on every other page, and apply opposite it, so the
// two presses - one that leaves the page unchanged and one that changes it -
// are never confused for each other
constexpr float BackX = 0.05f;
constexpr float BackY = 0.05f;
constexpr float ApplyX = 0.9f;
constexpr float ApplyY = 0.9f;
}  // namespace

BaseOptionState::BaseOptionState(EventQueue* CallBacksQueue,
                                 AppState StateOwned, InputDevice* PageDevice,
                                 int Owner, std::string PageName,
                                 std::string ConfigInstanceName)
    : BaseState(CallBacksQueue, StateOwned, Owner),
      Device(PageDevice),
      Page(std::move(PageName)),
      InstanceName(std::move(ConfigInstanceName)),
      BackButton(nullptr),
      ApplyButton(nullptr),
      BoundControls(0) {}

// the buttons are built before the settings so a page's own controls are laid
// out under them rather than around them
void BaseOptionState::OnInit() {
  const int Owner = GetOwner();

  BackButton = CreateElement<GenericButton>(
      PageElementName("back"), "back", std::vector<float>{BackX, BackY},
      Device,
      [this, Owner](EventQueue& Queue, float MouseX, float MouseY) {
        Queue.Enqueue(ChangeStateEvent{GetBackState(), Owner});
      },
      AppQueue, Owner);

  ApplyButton = CreateElement<GenericButton>(
      PageElementName("apply"), "apply", std::vector<float>{ApplyX, ApplyY},
      Device,
      [this](EventQueue& Queue, float MouseX, float MouseY) {
        ApplyStagedSettings();
      },
      AppQueue, Owner);

  OnBindSettings();
}

void BaseOptionState::OnEnter() {
  RefreshControls();
  OnPageEnter();
}

// an edit that was never applied is not a setting, so it does not survive the
// page being left. the controls are put back on the stored values on the way
// in rather than here, since a config can also be written from elsewhere
void BaseOptionState::OnExit() {
  for (const std::pair<const std::string, std::unique_ptr<ConfigManager>>&
           Entry : Configs) {
    Entry.second->DiscardStagedValues();
  }

  OnPageExit();
}

// applying is the only thing that writes a file, and the only thing that says
// so. a config nobody edited is left alone rather than being rewritten and
// announced, so a reader is never made to reload for nothing
void BaseOptionState::ApplyStagedSettings() {
  for (const std::pair<const std::string, std::unique_ptr<ConfigManager>>&
           Entry : Configs) {
    ConfigManager* Managed = Entry.second.get();

    if (!Managed->HasStagedValues()) {
      continue;
    }

    Managed->ApplyStagedValues();

    // read the file back, so this manager holds exactly what any other reader
    // is about to load rather than what it merged
    Managed->LoadOrReload();

    AppQueue->Enqueue(ConfigAppliedEvent{Managed->GetConfigName(),
                                         Managed->GetInstanceName(),
                                         GetOwner()});
  }

  // a track is continuous and plenty of the keys behind one are not, so what
  // was written is not always exactly where the control was dragged to. the
  // controls are put back on the written values, so the page is showing the
  // setting rather than the gesture
  RefreshControls();

  PageReporter.Dispatch();
}

void BaseOptionState::RefreshControls() {
  for (const std::function<void()>& Refresher : Refreshers) {
    Refresher();
  }
}

void BaseOptionState::AddRefresher(std::function<void()> Refresher) {
  if (Refresher) {
    Refreshers.push_back(std::move(Refresher));
  }
}

std::vector<float> BaseOptionState::TakeNextSlot() {
  const int Column = BoundControls / RowsPerColumn;
  const int Row = BoundControls % RowsPerColumn;
  BoundControls++;

  return std::vector<float>{
      FirstColumnX + (ColumnSpacing * static_cast<float>(Column)),
      FirstRowY + (RowSpacing * static_cast<float>(Row))};
}

std::vector<float> BaseOptionState::SliderSize() {
  return std::vector<float>{SliderWidth, SliderHeight};
}

std::string BaseOptionState::PageElementName(
    const std::string& ElementSuffix) const {
  return Page + "_" + ElementSuffix;
}

InputDevice* BaseOptionState::GetDevice() const { return Device; }

ConfigManager* BaseOptionState::Config(const std::string& ConfigBaseName) {
  return Config(ConfigBaseName, InstanceName);
}

ConfigManager* BaseOptionState::Config(const std::string& ConfigBaseName,
                                       const std::string& ConfigInstanceName) {
  // both names are what identify a file, so both have to be in the key - one
  // player's InputSettings is not the application's
  const std::string Key = ConfigBaseName + "|" + ConfigInstanceName;

  std::unordered_map<std::string, std::unique_ptr<ConfigManager>>::iterator
      Found = Configs.find(Key);

  if (Found != Configs.end()) {
    return Found->second.get();
  }

  std::unique_ptr<ConfigManager> Created = std::make_unique<ConfigManager>(
      ConfigBaseName, &PageReporter, ConfigInstanceName);

  ConfigManager* Managed = Created.get();
  Configs.emplace(Key, std::move(Created));

  PageReporter.Dispatch();
  return Managed;
}

GenericSlider* BaseOptionState::BindComponentSlider(
    std::string ElementName, std::string LabelText,
    const std::string& ConfigBaseName, std::string Key, size_t Component,
    float Minimum, float Maximum) {
  return BindComponentSlider(std::move(ElementName), std::move(LabelText),
                             Config(ConfigBaseName), std::move(Key), Component,
                             Minimum, Maximum);
}

GenericSlider* BaseOptionState::BindComponentSlider(
    std::string ElementName, std::string LabelText, ConfigManager* Target,
    std::string Key, size_t Component, float Minimum, float Maximum) {
  // reading one component out of a key that may be missing, malformed or
  // shorter than the component asked for. the floor of the range is the least
  // wrong answer in all three cases, and is what the slider would show anyway
  const std::function<float()> ReadComponent = [Target, Key, Component,
                                                Minimum]() {
    const std::vector<float> Stored =
        Target->GetStagedOrStored<std::vector<float>>(Key);

    return (Component < Stored.size()) ? Stored[Component] : Minimum;
  };

  std::shared_ptr<GenericSlider*> Handle =
      std::make_shared<GenericSlider*>(nullptr);

  const std::string Label = LabelText;
  const float Stored = ReadComponent();

  GenericSlider* Created = CreateElement<GenericSlider>(
      PageElementName(ElementName),
      Label + ": " + fmt::format("{:.2f}", Stored), TakeNextSlot(),
      SliderSize(), Minimum, Maximum, Device,
      [Handle, Target, Key, Label, Component, Minimum](EventQueue& Queue,
                                                       float Value) {
        // the key is the whole array, so it is read, edited and staged whole -
        // staging one number would replace the array with a number
        std::vector<float> Edited =
            Target->GetStagedOrStored<std::vector<float>>(Key);

        if (Edited.size() <= Component) {
          Edited.resize(Component + 1, Minimum);
        }

        Edited[Component] = Value;
        Target->StageValue<std::vector<float>>(Key, Edited);

        (*Handle)->SetLabelText(Label + ": " + fmt::format("{:.2f}", Value));
      },
      AppQueue, GetOwner());

  *Handle = Created;
  Created->SetValue(Stored);

  AddRefresher([Created, Label, ReadComponent]() {
    const float Current = ReadComponent();
    Created->SetValue(Current);
    Created->SetLabelText(Label + ": " + fmt::format("{:.2f}", Current));
  });

  PageReporter.Dispatch();
  return Created;
}

GenericButton* BaseOptionState::BindToggle(std::string ElementName,
                                           std::string LabelText,
                                           const std::string& ConfigBaseName,
                                           std::string Key) {
  return BindToggle(std::move(ElementName), std::move(LabelText),
                    Config(ConfigBaseName), std::move(Key));
}

GenericButton* BaseOptionState::BindToggle(std::string ElementName,
                                           std::string LabelText,
                                           ConfigManager* Target,
                                           std::string Key) {
  std::shared_ptr<GenericButton*> Handle =
      std::make_shared<GenericButton*>(nullptr);

  const std::string Label = LabelText;
  const bool Stored = Target->GetValueOrDefault<bool>(Key);

  GenericButton* Created = CreateElement<GenericButton>(
      PageElementName(ElementName), ToggleCaption(Label, Stored),
      TakeNextSlot(), Device,
      [Handle, Target, Key, Label](EventQueue& Queue, float MouseX,
                                   float MouseY) {
        // flipping the staged value rather than the stored one, so a toggle
        // pressed twice before applying really does end up where it started
        const bool Flipped = !Target->GetStagedOrStored<bool>(Key);

        Target->StageValue<bool>(Key, Flipped);
        (*Handle)->SetText(ToggleCaption(Label, Flipped));
      },
      AppQueue, GetOwner());

  *Handle = Created;

  AddRefresher([Created, Target, Key, Label]() {
    Created->SetText(
        ToggleCaption(Label, Target->GetValueOrDefault<bool>(Key)));
  });

  PageReporter.Dispatch();
  return Created;
}

std::string BaseOptionState::ToggleCaption(const std::string& LabelText,
                                           bool Value) {
  return LabelText + ": " + (Value ? "on" : "off");
}
