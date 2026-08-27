// Copyright (c) 2025 Henry Frodsham
#include "ConfigManager.h"

#include <string>

// responsible for loading and storing configs, if a custom file exists then use
// that. otherwise use the default
ConfigManager::ConfigManager(std::string BaseName,
                             ErrorReporter* ParentReporter,
                             std::string InstanceName)
    : ConfigName(BaseName),
      Reporter(ParentReporter),
      NameExtension(InstanceName),
      StagedValues(nlohmann::json::object()) {
  CustomPath = std::filesystem::path(std::filesystem::current_path()) /
               "config" / "custom" / (ConfigName + NameExtension + ".json");
  DefaultPath = std::filesystem::path(std::filesystem::current_path()) /
                "config" / "default" / (ConfigName + ".json");
  LoadOrReload();
}

bool ConfigManager::HasStagedValues() const {
  return StagedValues.is_object() && !StagedValues.empty();
}

// staged edits are an override of an override, so they are merged over the
// custom values rather than replacing them - a page only ever stages the
// handful of keys it actually has controls for
void ConfigManager::ApplyStagedValues() {
  if (!HasStagedValues()) {
    return;
  }

  // the first edit a config ever gets is what makes its custom file, so the
  // file is brought into existence here rather than being expected to already
  // be there
  EnsureCustomFileExists();

  if (!CustomValues.is_object()) {
    CustomValues = nlohmann::json::object();
  }

  CustomValues.merge_patch(StagedValues);
  StagedValues = nlohmann::json::object();

  SaveFiles();
}

void ConfigManager::DiscardStagedValues() {
  StagedValues = nlohmann::json::object();
}

// a key is real if either file knows about it. the default file is the one
// that defines what settings exist, and the custom file only ever overrides
// some of them, so a key missing from the custom file is not a missing key
bool ConfigManager::KeyExists(const std::string& Key) const {
  return (CustomValues.is_object() && CustomValues.contains(Key)) ||
         (DefaultValues.is_object() && DefaultValues.contains(Key));
}

const std::string& ConfigManager::GetConfigName() const { return ConfigName; }

const std::string& ConfigManager::GetInstanceName() const {
  return NameExtension;
}

void ConfigManager::LoadOrReload() {
  if (!std::filesystem::exists(CustomPath)) {
    Reporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::NO_CUSTOM_JSON,
        fmt::format("the custom json file with specified path {}, doesnt "
                    "exist, using default.",
                    CustomPath.string())));
  } else {
    CustomValues = OpenAndParse(CustomPath);
  }

  if (!std::filesystem::exists(DefaultPath)) {
    Reporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::JSON_FAILURE,
        fmt::format("no default config could be recovered at {}, unable to "
                    "provide any value ",
                    DefaultPath.string())));
  } else {
    DefaultValues = OpenAndParse(DefaultPath);
  }
}

nlohmann::json ConfigManager::OpenAndParse(std::filesystem::path JsonPath) {
  nlohmann::json Data = NULL;
  try {
    std::ifstream File(JsonPath);

    if (!File.is_open()) {
      Reporter->EnqueueError(ErrorDetail::CreateError(
          ErrorCode::JSON_UNOPENED_FILE,
          fmt::format("unable open the json file {}", JsonPath.string())));
      return Data;
    }

    Data = nlohmann::json::parse(File);
  } catch (const nlohmann::json::parse_error& e) {
    Reporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::JSON_PARSE_ERROR,
        fmt::format("unable to parse the json file {}, invalid syntax? \n "
                    "original error: {}",
                    JsonPath.string(), e.what())));
  } catch (std::exception& e) {
    Reporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::JSON_PARSE_ERROR,
        fmt::format("an unexpected error ({}) has occured whilst parsing json "
                    "file {} \n original error: {}",
                    typeid(e).name(), JsonPath.string(), e.what())));
  }

  return Data;
}

// only the custom file is ever written. the default file is the record of what
// a setting is supposed to be and what settings exist at all, so it stays
// exactly as it shipped - it is the thing a broken custom file falls back to,
// and it is no use as that if saving can overwrite it
void ConfigManager::SaveFiles() {
  if (CustomValues.is_null() || CustomValues.empty()) {
    return;
  }

  if (!EnsureCustomDirectory()) {
    return;
  }

  try {
    std::ofstream CustomFile(CustomPath);
    if (!CustomFile.is_open()) {
      Reporter->EnqueueError(ErrorDetail::CreateError(
          ErrorCode::JSON_UNOPENED_FILE,
          fmt::format("unable to open custom config file {} for writing",
                      CustomPath.string())));
      return;
    }

    // viewable format with 4 space indents
    CustomFile << CustomValues.dump(4);
    CustomFile.close();
  } catch (const std::exception& e) {
    Reporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::JSON_WRITE_ERROR,
        fmt::format(
            "failed to write custom config file {} \n original error: {}",
            CustomPath.string(), e.what())));
  }
}

// a config only has a custom file once something has been changed, so the
// first save has to make one. it is seeded with a full copy of the defaults
// rather than the single key that was edited, so what lands in config/custom
// is a complete, readable settings file somebody can open and edit by hand
// instead of a fragment that only makes sense next to the default
//
// a key still missing from it - one added to the default file by a later
// version - carries on falling back to the default, so seeding does not
// freeze a config at the shape it had the first time it was written
void ConfigManager::EnsureCustomFileExists() {
  if (std::filesystem::exists(CustomPath)) {
    return;
  }

  if (!CustomValues.is_object() || CustomValues.empty()) {
    CustomValues = DefaultValues.is_object() ? DefaultValues
                                             : nlohmann::json::object();
  }

  SaveFiles();
}

bool ConfigManager::EnsureCustomDirectory() {
  if (CustomPath.parent_path().empty()) {
    return true;
  }

  try {
    std::filesystem::create_directories(CustomPath.parent_path());
  } catch (const std::filesystem::filesystem_error& e) {
    Reporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::JSON_WRITE_ERROR,
        fmt::format("failed to create the custom config directory {} \n "
                    "original error: {}",
                    CustomPath.parent_path().string(), e.what())));
    return false;
  }

  return true;
}
