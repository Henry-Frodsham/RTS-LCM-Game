//Copyright © 2025 Henry Frodsham
#pragma once
#include <string>
#include <array>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <format>

// 5 level system, stores how "serious" an error is
enum ErrorLevel : uint16_t {
	TRACE = 0,
	DEBUG = 1,
	INFO = 2,
	WARNING = 3, // unexpected event but not critical
	ERR = 4, // unexpected event but recoverable
	FATAL = 5 // unexpected event and unrecoverable
};

// codes unique to an error, first digit denotes severity. from range 0000 - 5000
enum ErrorCode : uint32_t {
	//trace
	//debug
	//info
	RENDER_WINDOW_CLOSED = 2001,
	SDL_DEAD_DEVICE_ID = 2002,
	SDL_HANDLER_ID_SUCCESS = 2003,
	SDL_HANDLER_NEW_REG = 2004,
	SDL_NO_CONNECTED_CONTROLLERS = 2005,
	//warning
	BAD_SDL_ID_ON_REQUEST = 3001,
	//error
	UNSET_INPUT_LISTENER_QUEUE = 4001,
	SDL_CONTROLLER_FAILED_INIT = 4002,
	//fatal
	OGRE_NO_AVAILABLE_RENDER_SYSTEM = 5001,
	SDL_FAILED_INIT = 5002,
	SDL_FAILED_BIND = 5003
};

// general information on an error
struct Error {
	ErrorCode Code;
	ErrorLevel Level;
	std::string Message;
	std::string Origin;

	Error(ErrorCode ECode, ErrorLevel ELevel,
		std::string EMessage, std::string EOrigin)
		: Code(ECode), Level(ELevel), 
		Message(EMessage), Origin(EOrigin){
	}
};

// handleable errors
// inherit from Error to still allow logging 
// but contains extra information for error handling

struct DeadDeviceIdError : Error {
	SDL_Joystick* JoyStick;
	Sint32 SupposedId;

	DeadDeviceIdError(SDL_Joystick* Joy, Sint32 ID)
		: Error(ErrorCode::SDL_DEAD_DEVICE_ID,
			    ErrorLevel::INFO,
			    "SDL picked up an unregistered device ID. handling...",
			    "InputListener")
		,JoyStick(Joy)
		,SupposedId(ID){
	}
};




//stores a list of known potential errors, along with information for the end user
namespace ErrorDetail {
	const std::unordered_map<ErrorCode, Error> ErrorManifest = {
		//trace
		//debug
		//info
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
			 "there currently arent any controllers recognised by SDL.", "InputListener"}},


		//warning
		{ErrorCode::BAD_SDL_ID_ON_REQUEST,
			{ErrorCode::BAD_SDL_ID_ON_REQUEST, ErrorLevel::WARNING,
			 "a dead SDL ID was used when requesting an input device", "InputListener"}},

		//error
		{ErrorCode::UNSET_INPUT_LISTENER_QUEUE,
		   {ErrorCode::UNSET_INPUT_LISTENER_QUEUE, ErrorLevel::ERR,
			"An input device has no associated listener Queue", "InputListener"}},

		{ErrorCode::SDL_CONTROLLER_FAILED_INIT,
			{ErrorCode::SDL_CONTROLLER_FAILED_INIT, ErrorLevel::ERR,
			 "a controller failed init", "InputListener"}},
		//fatal
		{ErrorCode::OGRE_NO_AVAILABLE_RENDER_SYSTEM,
			{ErrorCode::OGRE_NO_AVAILABLE_RENDER_SYSTEM, ErrorLevel::FATAL,
			 "No available renders found/accessible", "RenderSystem"}},
		{ErrorCode::SDL_FAILED_INIT,
			{ErrorCode::SDL_FAILED_INIT, ErrorLevel::FATAL,
			 "SDL init failed", "RenderSystem"}},
		
		{ErrorCode::SDL_FAILED_BIND,
			{ErrorCode::SDL_FAILED_BIND, ErrorLevel::FATAL,
			 "SDL failed bind to primary window", "RenderSystem"}}
		

	};
}