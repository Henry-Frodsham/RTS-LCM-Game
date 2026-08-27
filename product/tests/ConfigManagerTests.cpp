// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <filesystem>  // NOLINT(build/c++17)
#include <fstream>
#include <iterator>
#include <string>

#include "ConfigManager.h"
#include "ErrorReporter.h"

namespace {
// the custom files these tests write are real files in the running
// directory's config folder, so a test that makes one takes it away again
std::filesystem::path DefaultPathFor(const std::string& FileName) {
  return std::filesystem::path(std::filesystem::current_path()) / "config" /
         "default" / (FileName + ".json");
}

std::string ReadWhole(const std::filesystem::path& Path) {
  std::ifstream File(Path);
  return std::string((std::istreambuf_iterator<char>(File)),
                     std::istreambuf_iterator<char>());
}

std::filesystem::path CustomPathFor(const std::string& FileName) {
  return std::filesystem::path(std::filesystem::current_path()) / "config" /
         "custom" / (FileName + ".json");
}

void RemoveCustomFile(const std::string& FileName) {
  std::error_code Ignored;
  std::filesystem::remove(CustomPathFor(FileName), Ignored);
}
}  // namespace

TEST_CASE("ConfigManager - reads real values from the GlobeSettings default "
         "config") {
  ErrorReporter Reporter;
  ConfigManager Config("GlobeSettings", &Reporter, "_cm_test_1");

  CHECK(Config.GetValueOrDefault<int>("NumSubdivisions") == 100);
  CHECK(Config.GetValueOrDefault<int>("CreationSeed") == 5555);
  CHECK(Config.GetValueOrDefault<float>("PlanetRadius") == 0.5f);
}

TEST_CASE("ConfigManager - reads real values from the VideoSettings default "
         "config") {
  ErrorReporter Reporter;
  ConfigManager Config("VideoSettings", &Reporter, "_cm_test_2");

  CHECK(Config.GetValueOrDefault<int>("WindowWidth") == 1920);
  CHECK(Config.GetValueOrDefault<bool>("FullScreen") == false);
  CHECK(Config.GetValueOrDefault<std::string>("WindowName") == "RTS LCM GAME");
}

TEST_CASE("ConfigManager - a missing key returns a default-constructed "
         "value instead of throwing") {
  ErrorReporter Reporter;
  ConfigManager Config("GlobeSettings", &Reporter, "_cm_test_3");
  int Missing = 0;
  CHECK_NOTHROW(Missing = Config.GetValueOrDefault<int>("ThisKeyDoesNotExist"));
  CHECK(Missing == 0);
}

TEST_CASE("ConfigManager - construction with no matching custom override "
         "still succeeds") {
  ErrorReporter Reporter;
  CHECK_NOTHROW(ConfigManager("GlobeSettings", &Reporter, "_cm_test_4"));
}

TEST_CASE("ConfigManager - UpdateValue on a key absent from CustomValues "
         "does not throw") {
  ErrorReporter Reporter;
  ConfigManager Config("GlobeSettings", &Reporter, "_cm_test_5");
  CHECK_NOTHROW(Config.UpdateValue<int>("NumSubdivisions", 200));

  // a key the default file defines is a real setting even before any custom
  // file exists, so writing it for the first time is what is expected to have
  // happened rather than an error
  CHECK(Config.GetValueOrDefault<int>("NumSubdivisions") == 200);

  RemoveCustomFile("GlobeSettings_cm_test_5");
}

TEST_CASE("ConfigManager - a staged value is not visible and is not written "
         "until it is applied") {
  ErrorReporter Reporter;
  ConfigManager Config("GlobeSettings", &Reporter, "_cm_test_6");

  Config.StageValue<int>("NumSubdivisions", 200);

  CHECK(Config.HasStagedValues());
  CHECK(Config.GetValueOrDefault<int>("NumSubdivisions") == 100);
  CHECK(Config.GetStagedOrStored<int>("NumSubdivisions") == 200);
  CHECK_FALSE(
      std::filesystem::exists(CustomPathFor("GlobeSettings_cm_test_6")));

  Config.ApplyStagedValues();

  CHECK_FALSE(Config.HasStagedValues());
  CHECK(Config.GetValueOrDefault<int>("NumSubdivisions") == 200);
  CHECK(std::filesystem::exists(CustomPathFor("GlobeSettings_cm_test_6")));

  RemoveCustomFile("GlobeSettings_cm_test_6");
}

TEST_CASE("ConfigManager - applying only writes the keys that were staged") {
  ErrorReporter Reporter;
  ConfigManager Config("GlobeSettings", &Reporter, "_cm_test_7");

  Config.StageValue<int>("CreationSeed", 42);
  Config.ApplyStagedValues();

  // the untouched keys still answer, out of the default file, rather than
  // being blanked by a custom file that only knows about one of them
  CHECK(Config.GetValueOrDefault<int>("CreationSeed") == 42);
  CHECK(Config.GetValueOrDefault<int>("NumSubdivisions") == 100);
  CHECK(Config.GetValueOrDefault<float>("PlanetRadius") == 0.5f);

  RemoveCustomFile("GlobeSettings_cm_test_7");
}

TEST_CASE("ConfigManager - discarded edits never reach the file") {
  ErrorReporter Reporter;
  ConfigManager Config("GlobeSettings", &Reporter, "_cm_test_8");

  Config.StageValue<int>("NumSubdivisions", 200);
  Config.DiscardStagedValues();

  CHECK_FALSE(Config.HasStagedValues());
  CHECK(Config.GetStagedOrStored<int>("NumSubdivisions") == 100);

  Config.ApplyStagedValues();
  CHECK_FALSE(
      std::filesystem::exists(CustomPathFor("GlobeSettings_cm_test_8")));
}

TEST_CASE("ConfigManager - applying never writes the default file") {
  ErrorReporter Reporter;
  const std::string Before = ReadWhole(DefaultPathFor("GlobeSettings"));

  ConfigManager Config("GlobeSettings", &Reporter, "_cm_test_10");
  Config.StageValue<int>("NumSubdivisions", 200);
  Config.ApplyStagedValues();

  // the default file is what a broken or missing custom file falls back to,
  // so it has to come out of a save byte for byte what it went in as
  CHECK(ReadWhole(DefaultPathFor("GlobeSettings")) == Before);

  RemoveCustomFile("GlobeSettings_cm_test_10");
}

TEST_CASE("ConfigManager - the custom file is created seeded with the whole "
         "default, not just the edited key") {
  ErrorReporter Reporter;
  ConfigManager Config("GlobeSettings", &Reporter, "_cm_test_11");

  REQUIRE_FALSE(
      std::filesystem::exists(CustomPathFor("GlobeSettings_cm_test_11")));

  Config.StageValue<int>("NumSubdivisions", 200);
  Config.ApplyStagedValues();

  const nlohmann::json Written = nlohmann::json::parse(
      ReadWhole(CustomPathFor("GlobeSettings_cm_test_11")));

  CHECK(Written["NumSubdivisions"] == 200);
  CHECK(Written.contains("CreationSeed"));
  CHECK(Written.contains("PlanetRadius"));

  RemoveCustomFile("GlobeSettings_cm_test_11");
}

TEST_CASE("ConfigManager - EnsureCustomFileExists makes a file without any "
         "edit and leaves an existing one alone") {
  ErrorReporter Reporter;
  ConfigManager Config("GlobeSettings", &Reporter, "_cm_test_12");

  Config.EnsureCustomFileExists();
  REQUIRE(std::filesystem::exists(CustomPathFor("GlobeSettings_cm_test_12")));

  const std::string Seeded =
      ReadWhole(CustomPathFor("GlobeSettings_cm_test_12"));

  Config.EnsureCustomFileExists();
  CHECK(ReadWhole(CustomPathFor("GlobeSettings_cm_test_12")) == Seeded);

  RemoveCustomFile("GlobeSettings_cm_test_12");
}

TEST_CASE("ConfigManager - a key neither file defines cannot be staged") {
  ErrorReporter Reporter;
  ConfigManager Config("GlobeSettings", &Reporter, "_cm_test_9");

  Config.StageValue<int>("ThisKeyDoesNotExist", 1);

  CHECK_FALSE(Config.HasStagedValues());
  CHECK_FALSE(Config.KeyExists("ThisKeyDoesNotExist"));
  CHECK(Config.KeyExists("NumSubdivisions"));
}
