// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <SDL2/SDL.h>
#include <fmt/format.h>

#include <array>
#include <string>
#include <unordered_map>
#include <utility>

// 5 level system, stores how "serious" an error is
enum ErrorLevel : uint16_t {
  TRACE = 0,
  DEBUG = 1,
  INFO = 2,
  WARNING = 3,  // unexpected event but not critical
  ERR = 4,      // unexpected event but recoverable
  FATAL = 5     // unexpected event and unrecoverable
};

// codes unique to an error, first digit denotes severity. from range 0000 -
// 5000
enum ErrorCode : uint32_t {
  // trace
  // debug
  // info
  RENDER_WINDOW_CLOSED = 2001,
  SDL_DEAD_DEVICE_ID = 2002,
  SDL_HANDLER_ID_SUCCESS = 2003,
  SDL_HANDLER_NEW_REG = 2004,
  SDL_NO_CONNECTED_CONTROLLERS = 2005,
  SDL_CONTROLLER_DISCONNECT = 2006,
  SDL_CONTOLLER_CONNECT = 2007,
  NO_CUSTOM_JSON = 2008,
  // warning
  BAD_SDL_ID_ON_REQUEST = 3001,
  // error
  UNSET_INPUT_LISTENER_QUEUE = 4001,
  SDL_CONTROLLER_FAILED_INIT = 4002,
  MATERIAL_NOT_FOUND = 4003,
  OVERLAY_NOT_FOUND = 4004,
  ELEMENT_NOT_FOUND = 4005,
  JSON_PARSE_ERROR = 4006,
  JSON_FILE_NOT_FOUND = 4007,
  JSON_UNOPENED_FILE = 4008,
  CUSTOM_JSON_VALUE_FAILURE = 4009,
  JSON_WRITE_ERROR = 4010,
  ECS_WRONG_CREATION_ORDER = 4011,
  SCENE_NODE_DOESNT_EXIST = 4012,
  INSTANCE_REQUEST_NO_DEVICE = 4013,
  BAD_VIEWPORT_CONTROL_REQUEST = 4014,
  ATTEMPT_TO_ASSIGN_VIEWPORT_TO_UNSET_OVERLAY = 4015,
  ATTEMPT_ASSIGN_OVERLAY_TO_UNSET_VIEWPORT = 4016,
  INVALID_ORDER_OF_OPERATIONS = 4017,
  ILLEGAL_OVERLAY_COMMAND = 4018,
  OVERLAY_MISSING_INFO = 4019,
  CALLBACK_FAILED = 4020,
  RANGE_CHECK_TOO_EARLY = 4021,
  RANGE_CHECK_FAILED = 4022,
  ATTACK_LOGIC_EARLY = 4023,
  ATTACK_LOGIC_FAILED = 4024,
  // fatal
  OGRE_NO_AVAILABLE_RENDER_SYSTEM = 5001,
  SDL_FAILED_INIT = 5002,
  SDL_FAILED_BIND = 5003,
  OVERLAY_UNITIALISED = 5004,
  JSON_FAILURE = 5005,
  DEFAULT_JSON_VALUE_FAILURE = 5006,
  JSON_VALUE_MISSING_KEYS = 5007,
  TRANSLATOR_NO_DEVICE = 5008,
  IDENTICAL_OVERLAY = 5009,
  UNRECOGNISED = 5999
};

// general information on an error
struct Error {
  ErrorCode Code;
  ErrorLevel Level;
  std::string Message;
  std::string Origin;

  Error(ErrorCode ECode, ErrorLevel ELevel, std::string EMessage,
        std::string EOrigin)
      : Code(ECode), Level(ELevel), Message(EMessage), Origin(EOrigin) {}
};

// stores a list of known potential errors, along with information for the end
// user
namespace ErrorDetail {
const std::unordered_map<ErrorCode, Error> ErrorManifest = {
    // trace
    // debug
    // info
    {ErrorCode::RENDER_WINDOW_CLOSED,
     {ErrorCode::RENDER_WINDOW_CLOSED, ErrorLevel::INFO,
      "The render window has closed, exiting.", "RenderSystem"}},

    {ErrorCode::SDL_HANDLER_ID_SUCCESS,
     {ErrorCode::SDL_HANDLER_ID_SUCCESS, ErrorLevel::INFO,
      "device succesfully reconnected", "InputListener"}},

    {ErrorCode::SDL_HANDLER_NEW_REG,
     {ErrorCode::SDL_HANDLER_NEW_REG, ErrorLevel::INFO,
      "A new device has succesfully be registered", "InputListener"}},

    {ErrorCode::SDL_NO_CONNECTED_CONTROLLERS,
     {ErrorCode::SDL_NO_CONNECTED_CONTROLLERS, ErrorLevel::INFO,
      "there currently arent any controllers recognised by SDL.",
      "InputListener"}},

    {ErrorCode::SDL_CONTROLLER_DISCONNECT,
     {ErrorCode::SDL_CONTROLLER_DISCONNECT, ErrorLevel::INFO,
      "a controller was disconnected.", "InputListener"}},

    {ErrorCode::SDL_CONTOLLER_CONNECT,
     {ErrorCode::SDL_CONTOLLER_CONNECT, ErrorLevel::INFO,
      "a new controller was connected", "InputListener"}},

    {ErrorCode::NO_CUSTOM_JSON,
     {ErrorCode::NO_CUSTOM_JSON, ErrorLevel::INFO,
      "no custom config was found, continuing and using default",
      "ConfigManager"}},

    // warning
    {ErrorCode::BAD_SDL_ID_ON_REQUEST,
     {ErrorCode::BAD_SDL_ID_ON_REQUEST, ErrorLevel::WARNING,
      "a dead SDL ID was used when requesting an input device",
      "InputListener"}},

    {ErrorCode::SDL_DEAD_DEVICE_ID,
     {ErrorCode::SDL_DEAD_DEVICE_ID, ErrorLevel::WARNING,
      "a dead SDL ID was used when requesting an input device",
      "InputListener"}},
    // error
    {ErrorCode::UNSET_INPUT_LISTENER_QUEUE,
     {ErrorCode::UNSET_INPUT_LISTENER_QUEUE, ErrorLevel::ERR,
      "An input device has no associated listener Queue", "InputListener"}},

    {ErrorCode::SDL_CONTROLLER_FAILED_INIT,
     {ErrorCode::SDL_CONTROLLER_FAILED_INIT, ErrorLevel::ERR,
      "a controller failed init", "InputListener"}},

    {ErrorCode::MATERIAL_NOT_FOUND,
     {ErrorCode::MATERIAL_NOT_FOUND, ErrorLevel::ERR,
      "a requested material was not found", "Generic"}},

    {ErrorCode::OVERLAY_NOT_FOUND,
     {ErrorCode::OVERLAY_NOT_FOUND, ErrorLevel::ERR,
      "unfound overlay name in overlay event", "OverlayController"}},

    {ErrorCode::ELEMENT_NOT_FOUND,
     {ErrorCode::ELEMENT_NOT_FOUND, ErrorLevel::ERR,
      "unfound overlay element in overlay edit event", "OverlayController"}},

    {ErrorCode::JSON_FILE_NOT_FOUND,
     {ErrorCode::JSON_FILE_NOT_FOUND, ErrorLevel::ERR,
      "a json file wasnt found at the specified location", "ConfigManager"}},

    {ErrorCode::JSON_PARSE_ERROR,
     {ErrorCode::JSON_PARSE_ERROR, ErrorLevel::ERR,
      "a json file contains invalid syntax", "ConfigManager"}},

    {ErrorCode::JSON_UNOPENED_FILE,
     {ErrorCode::JSON_UNOPENED_FILE, ErrorLevel::ERR,
      "a json file was unable to be opened", "ConfigManager"}},

    {ErrorCode::CUSTOM_JSON_VALUE_FAILURE,
     {ErrorCode::CUSTOM_JSON_VALUE_FAILURE, ErrorLevel::ERR,
      "a custom value couldnt be retrieved from the requested key",
      "ConfigManager"}},

    {ErrorCode::JSON_WRITE_ERROR,
     {ErrorCode::JSON_WRITE_ERROR, ErrorLevel::ERR,
      "a json file couldnt be written to", "ConfigManager"}},

    {ErrorCode::ECS_WRONG_CREATION_ORDER,
     {ErrorCode::ECS_WRONG_CREATION_ORDER, ErrorLevel::ERR,
      "an ECS component couldnt be created because the prerequisite component "
      "could not be found",
      "ECSFactory"}},

    {ErrorCode::SCENE_NODE_DOESNT_EXIST,
     {ErrorCode::SCENE_NODE_DOESNT_EXIST, ErrorLevel::ERR,
      "the requested scene node didnt exist", "RenderSystem"}},

    {ErrorCode::INSTANCE_REQUEST_NO_DEVICE,
     {ErrorCode::INSTANCE_REQUEST_NO_DEVICE, ErrorLevel::ERR,
      "creating a new instance failed because the request didnt contain a "
      "valid device",
      "InstanceOverseer"}},

    {ErrorCode::BAD_VIEWPORT_CONTROL_REQUEST,
     {ErrorCode::BAD_VIEWPORT_CONTROL_REQUEST, ErrorLevel::ERR,
      "a request made by an instance thread to control their viewport was "
      "invalid",
      "InstanceOverseer"}},

    {ErrorCode::ATTEMPT_ASSIGN_OVERLAY_TO_UNSET_VIEWPORT,
     {ErrorCode::ATTEMPT_ASSIGN_OVERLAY_TO_UNSET_VIEWPORT, ErrorLevel::ERR,
      "couldnt assign an overlay to a viewport because the overlay was a "
      "nullptr",
      "ViewPortUpdateListener"}},

    {ErrorCode::ATTEMPT_TO_ASSIGN_VIEWPORT_TO_UNSET_OVERLAY,
     {ErrorCode::ATTEMPT_TO_ASSIGN_VIEWPORT_TO_UNSET_OVERLAY, ErrorLevel::ERR,
      "couldnt assign an viewport to an overlay because the viewport was a "
      "nullptr",
      "ViewPortUpdateListener"}},
    {ErrorCode::INVALID_ORDER_OF_OPERATIONS,
     {ErrorCode::INVALID_ORDER_OF_OPERATIONS, ErrorLevel::ERR,
      "A chain transaction has executed in the wrong order, requesting a "
      "resource before it was available",
      "General"}},
    {ErrorCode::OVERLAY_MISSING_INFO,
     {ErrorCode::OVERLAY_MISSING_INFO, ErrorLevel::ERR,
      "an overlay is missing info, does it exist?", "OverlayController"}},
    {ErrorCode::CALLBACK_FAILED,
     {ErrorCode::CALLBACK_FAILED, ErrorLevel::ERR,
      "An Overlay's interaction callback failed", "OverlayController"}},
    {ErrorCode::RANGE_CHECK_TOO_EARLY,
     {ErrorCode::RANGE_CHECK_TOO_EARLY, ErrorLevel::ERR,
      "Attempted a range check on an entity without an initialised SceneNode",
      "WorldManager"}},
    {ErrorCode::RANGE_CHECK_FAILED,
     {ErrorCode::RANGE_CHECK_FAILED, ErrorLevel::ERR,
      "Failed to check for nearby entities due to an unknown error",
      "WorldManager"}},
    {ErrorCode::ATTACK_LOGIC_EARLY,
     {ErrorCode::ATTACK_LOGIC_EARLY, ErrorLevel::ERR,
      "Failed to attack a nearby entity due to the entity not having a valid SceneNode",
      "WorldManager"}},
    {ErrorCode::ATTACK_LOGIC_FAILED,
     {ErrorCode::ATTACK_LOGIC_FAILED, ErrorLevel::ERR,
      "Failed to attack a nearby entity due to an unknown error",
      "WorldManager"}},
    {ErrorCode::CALLBACK_FAILED,
     {ErrorCode::CALLBACK_FAILED, ErrorLevel::ERR,
      "An Overlay's interaction callback failed", "OverlayController"}},
    // fatal
    {ErrorCode::OGRE_NO_AVAILABLE_RENDER_SYSTEM,
     {ErrorCode::OGRE_NO_AVAILABLE_RENDER_SYSTEM, ErrorLevel::FATAL,
      "No available renders found/accessible", "RenderSystem"}},

    {ErrorCode::SDL_FAILED_INIT,
     {ErrorCode::SDL_FAILED_INIT, ErrorLevel::FATAL, "SDL init failed",
      "RenderSystem"}},

    {ErrorCode::SDL_FAILED_BIND,
     {ErrorCode::SDL_FAILED_BIND, ErrorLevel::FATAL,
      "SDL failed bind to primary window", "RenderSystem"}},

    {ErrorCode::OVERLAY_UNITIALISED,
     {ErrorCode::OVERLAY_UNITIALISED, ErrorLevel::FATAL,
      "attempted overlay call before OverlaySystem is initialised",
      "OverlaySystem"}},

    {ErrorCode::JSON_FAILURE,
     {ErrorCode::JSON_FAILURE, ErrorLevel::FATAL,
      "failed to load custom config and failed to load default values as a "
      "fallback",
      "ConfigManager"}},

    {ErrorCode::DEFAULT_JSON_VALUE_FAILURE,
     {ErrorCode::DEFAULT_JSON_VALUE_FAILURE, ErrorLevel::FATAL,
      "a requested key couldnt be retrieved from either default or custom json "
      "maps",
      "ConfigManager"}},

    {ErrorCode::JSON_VALUE_MISSING_KEYS,
     {ErrorCode::JSON_VALUE_MISSING_KEYS, ErrorLevel::FATAL,
      "a requested key wasnt found in either custom or default maps",
      "ConfigManager"}},

    {ErrorCode::IDENTICAL_OVERLAY,
     {ErrorCode::IDENTICAL_OVERLAY, ErrorLevel::FATAL,
      "An attempt was made to create a duplicate overlay",
      "OverlayController"}},

    {ErrorCode::UNRECOGNISED,
     {ErrorCode::UNRECOGNISED, ErrorLevel::FATAL,
      "an unrecognised error has occured", "APPLICATION PANIC"}}};

// get default error from manifest
inline Error CreateError(ErrorCode ECode) { return ErrorManifest.at(ECode); }

// create error with custom message
inline Error CreateError(ErrorCode ECode, std::string CustomMessage) {
  const Error& DefaultErr = ErrorManifest.at(ECode);
  return Error(ECode, DefaultErr.Level, std::move(CustomMessage),
               DefaultErr.Origin);
}
}  // namespace ErrorDetail

// handleable errors
// inherit from Error to still allow logging
// but contains extra information for error handling

struct DeadDeviceIdError : Error {
  SDL_Joystick* JoyStick;
  Sint32 SupposedId;

  DeadDeviceIdError(SDL_Joystick* Joy, Sint32 ID)
      : Error(ErrorDetail::CreateError(ErrorCode::SDL_DEAD_DEVICE_ID)),
        JoyStick(Joy),
        SupposedId(ID) {}
  DeadDeviceIdError(SDL_Joystick* Joy, Sint32 ID, std::string CustomMessage)
      : Error(ErrorDetail::CreateError(ErrorCode::SDL_DEAD_DEVICE_ID,
                                       CustomMessage)),
        JoyStick(Joy),
        SupposedId(ID) {}
};
