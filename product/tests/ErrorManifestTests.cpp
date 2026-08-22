// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "ErrorManifest.h"

TEST_CASE("ErrorManifest - CreateError returns the manifest entry for an "
         "ERR level code") {
  Error E = ErrorDetail::CreateError(ErrorCode::MATERIAL_NOT_FOUND);
  CHECK(E.Code == ErrorCode::MATERIAL_NOT_FOUND);
  CHECK(E.Level == ErrorLevel::ERR);
  CHECK(E.Origin == "Generic");
}

TEST_CASE("ErrorManifest - CreateError with a custom message overrides the "
         "message but keeps level and origin") {
  Error Default = ErrorDetail::CreateError(ErrorCode::OVERLAY_NOT_FOUND);
  Error Custom =
      ErrorDetail::CreateError(ErrorCode::OVERLAY_NOT_FOUND, "custom text");

  CHECK(Custom.Message == "custom text");
  CHECK(Custom.Code == Default.Code);
  CHECK(Custom.Level == Default.Level);
  CHECK(Custom.Origin == Default.Origin);
}

TEST_CASE("ErrorManifest - an INFO level code resolves correctly") {
  Error E = ErrorDetail::CreateError(ErrorCode::RENDER_WINDOW_CLOSED);
  CHECK(E.Level == ErrorLevel::INFO);
  CHECK(E.Origin == "RenderSystem");
}

TEST_CASE("ErrorManifest - a WARNING level code resolves correctly") {
  Error E = ErrorDetail::CreateError(ErrorCode::BAD_SDL_ID_ON_REQUEST);
  CHECK(E.Level == ErrorLevel::WARNING);
}

TEST_CASE("ErrorManifest - a FATAL level code resolves correctly") {
  Error E = ErrorDetail::CreateError(ErrorCode::OGRE_NO_AVAILABLE_RENDER_SYSTEM);
  CHECK(E.Level == ErrorLevel::FATAL);
  CHECK(E.Origin == "RenderSystem");
}

TEST_CASE("ErrorManifest - DeadDeviceIdError defaults to the manifest's "
         "SDL_DEAD_DEVICE_ID entry") {
  DeadDeviceIdError E(nullptr, 42);
  CHECK(E.Code == ErrorCode::SDL_DEAD_DEVICE_ID);
  CHECK(E.Level == ErrorLevel::WARNING);
  CHECK(E.SupposedId == 42);
}
