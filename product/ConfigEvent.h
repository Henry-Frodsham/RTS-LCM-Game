// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <string>
#include <utility>

// one config file has just been rewritten by an options page and needs picking
// up again. anything that read values out of that file when it was built - an
// InputTranslator's sensitivities, the render system's window settings - is
// holding a copy that is now stale, so on hearing this it calls LoadOrReload
// on its own ConfigManager and re-reads whatever it cached. that is what makes
// an apply button hot rather than a restart
//
// ConfigName is the base name the file was built with ("InputSettings") and
// InstanceName the per instance suffix ConfigManager was given - empty for a
// config the whole application shares, otherwise the instance number, matching
// the convention InputTranslator already uses. a subscriber that only cares
// about its own player checks both, since every instance's file is announced
// the same way
//
// to pick a setting up, subscribe on the application bus and reload:
//
//   Bus->Subscribe<ConfigAppliedEvent>([this](const ConfigAppliedEvent& E) {
//     if (E.ConfigName != "InputSettings" ||
//         E.InstanceName != std::to_string(ThreadNumber)) {
//       return;
//     }
//     InputConfig->LoadOrReload();
//     CursorSensitivity =
//         InputConfig->GetValueOrDefault<float>("CursorSensitivity");
//   });
struct ConfigAppliedEvent {
  std::string ConfigName;
  std::string InstanceName;

  // the instance whose options page applied this, so a subscriber can ignore
  // somebody else's split screen page. StateOwnerAll for a page that is the
  // whole application's
  int Owner;

  ConfigAppliedEvent(std::string Config, std::string Instance, int AppliedBy)
      : ConfigName(std::move(Config)),
        InstanceName(std::move(Instance)),
        Owner(AppliedBy) {}
};
