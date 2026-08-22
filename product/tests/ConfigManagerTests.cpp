// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <string>

#include "ConfigManager.h"
#include "ErrorReporter.h"

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
}
