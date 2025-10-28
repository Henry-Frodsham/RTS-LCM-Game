//Copyright © 2025 Henry Frodsham
#pragma once
#include <string>
#include <array>

// 5 level system, stores how "serious" an error is
const enum ErrorLevel : uint16_t {
	TRACE = 0,
	DEBUG = 1,
	INFO = 2,
	WARNING = 3, // unexpected event but not critical
	ERROR = 4, // unexpected event but recoverable
	FATAL = 5 // unexpected event and unrecoverable
};

// general information on an error
const struct Error {
	ErrorLevel Level;
	ErrorCode Code;
	std::string Message;
	std::string Origin;
};

// codes unique to an error, first digit denotes severity. from range 0000 - 5000
const enum class ErrorCode : uint32_t {
	//trace
	//debug
	//info
	//warning
	//error
	//fatal
	OGRE_NO_AVAILABLE_RENDER_SYSTEM = 5001,
	SDL_FAILED_INIT = 5002
};


//stores a list of known potential errors, along with information for the end user
namespace ErrorDetail {
	Error ErrorManifest[] = {
		//trace
		//debug
		//info
		//warning
		//error
		//fatal
		{ErrorLevel::FATAL, ErrorCode::OGRE_NO_AVAILABLE_RENDER_SYSTEM, 
			"No available renders found/accessible", "RenderSystem"},
		{ErrorLevel::FATAL, ErrorCode::SDL_FAILED_INIT, "SDL init failed", "RenderSystem"}
	};
}