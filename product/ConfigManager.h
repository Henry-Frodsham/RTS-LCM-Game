// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <filesystem>  // NOLINT(build/c++17)
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>  // NOLINT(build/include_order)

#include "ErrorReporter.h"

// universal config class to handle any config file
// each instance is unique to a config file
// e.g InputTranslator will need an instance to control input sensitivity
class ConfigManager {
 public:
  // each instance of the game can have seperate configs for different things
  // however, not everything is seperated into instances
  // updating each ConfigManager is impractical so just use the callers reporter
  ConfigManager(std::string BaseName, ErrorReporter* ParentReporter,
                std::string InstanceName = "");

  // a universal retrieval function to get any value from the stored
  // nlohmann::json map template (return):
  //   DataType - the data type to return
  // param:
  //   Key - the json key to retrieve the value from
  template <typename DataType>
  DataType GetValueOrDefault(std::string Key) {
    if (CustomValues.contains(Key)) {
      try {
        return CustomValues[Key].get<DataType>();
      } catch (const std::exception& e) {
        Reporter->EnqueueError(ErrorDetail::CreateError(
            ErrorCode::CUSTOM_JSON_VALUE_FAILURE,
            fmt::format("failed to retrieve custom value from key {} in {}, "
                        "falling back to default \n original error : {}",
                        Key, ConfigName, e.what())));
      }
    }

    if (DefaultValues.contains(Key)) {
      try {
        return DefaultValues[Key].get<DataType>();
      } catch (const nlohmann::json::exception& e) {
        Reporter->EnqueueError(ErrorDetail::CreateError(
            ErrorCode::DEFAULT_JSON_VALUE_FAILURE,
            fmt::format("failed to retrieve default value from key {} in {} \n "
                        "original error : {}",
                        Key, ConfigName, e.what())));
        return DataType{};
      }
    }

    Reporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::JSON_VALUE_MISSING_KEYS,
        fmt::format("no corresponding values could be found for key {} in {}",
                    Key, ConfigName)));
    return DataType{};
  }

  // a universal function to update any value from a key to any DataType,
  // written to the custom file straight away
  // param:
  //  - DataType - the datatype of the value to update with
  //  - Key - the json key used to find the location to update
  template <typename DataType>
  void UpdateValue(std::string Key, DataType NewData) {
    // the custom file is an override of the default and usually does not exist
    // at all until something is changed, so a key only the default knows about
    // is still a key that can be written - anything stricter and no setting
    // could ever be saved for the first time
    if (KeyExists(Key)) {
      if (!CustomValues.is_object()) {
        CustomValues = nlohmann::json::object();
      }
      CustomValues[Key] = NewData;
      SaveFiles();
      return;
    }
    Reporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::JSON_VALUE_MISSING_KEYS,
        fmt::format("failed to update value key {} in {}", Key, ConfigName)));
  }

  // remember a value to be written the next time the settings are applied.
  // nothing reaches the file, and nothing that has already read the file sees
  // it, until ApplyStagedValues - so a half finished options page can be left
  // without any of its edits taking effect
  // param:
  //  - DataType - the datatype of the value to stage
  //  - Key - the json key the value will eventually be written to
  template <typename DataType>
  void StageValue(std::string Key, DataType NewData) {
    if (!KeyExists(Key)) {
      Reporter->EnqueueError(ErrorDetail::CreateError(
          ErrorCode::JSON_VALUE_MISSING_KEYS,
          fmt::format("failed to stage value key {} in {}, no such key exists "
                      "in the custom or the default file",
                      Key, ConfigName)));
      return;
    }
    StagedValues[Key] = NewData;
  }

  // the value a staged edit would give this key, or the stored one if it has
  // not been edited - what a control on an options page should be showing
  template <typename DataType>
  DataType GetStagedOrStored(std::string Key) {
    if (StagedValues.contains(Key)) {
      try {
        return StagedValues[Key].get<DataType>();
      } catch (const std::exception& e) {
        Reporter->EnqueueError(ErrorDetail::CreateError(
            ErrorCode::CUSTOM_JSON_VALUE_FAILURE,
            fmt::format("failed to retrieve staged value from key {} in {}, "
                        "falling back to the stored one \n original error : {}",
                        Key, ConfigName, e.what())));
      }
    }
    return GetValueOrDefault<DataType>(Key);
  }

  bool HasStagedValues() const;

  // write every staged edit into the custom file and forget them. nothing here
  // tells a reader to look again - that is the caller's to announce, since
  // only it knows who was reading this config
  void ApplyStagedValues();
  void DiscardStagedValues();

  bool KeyExists(const std::string& Key) const;

  // make the config/custom file for this config if there is not one yet,
  // seeded with a copy of the defaults. applying an edit already does this,
  // so it is only called directly by something that wants the file to exist
  // before anything has been changed
  void EnsureCustomFileExists();

  const std::string& GetConfigName() const;
  const std::string& GetInstanceName() const;

  void LoadOrReload();

  // writes the custom file only - the default is never written to
  void SaveFiles();

 private:
  nlohmann::json DefaultValues;
  nlohmann::json CustomValues;

  // edits waiting on an apply, kept apart from CustomValues so that discarding
  // them is just dropping this and never a reread of the file
  nlohmann::json StagedValues;

  ErrorReporter* Reporter;
  std::string ConfigName;
  std::string NameExtension;

  std::filesystem::path DefaultPath;
  std::filesystem::path CustomPath;

  nlohmann::json OpenAndParse(std::filesystem::path JsonPath);

  // config/custom does not ship with the game, so it has to be made before
  // the first file can be written into it
  bool EnsureCustomDirectory();
};
