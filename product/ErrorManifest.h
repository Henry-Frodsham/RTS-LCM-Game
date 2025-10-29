//Copyright © 2025 Henry Frodsham
#pragma once
#include <string>
#include <array>
#include <unordered_map>

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
	//warning
	//error
	//fatal
	OGRE_NO_AVAILABLE_RENDER_SYSTEM = 5001,
	SDL_FAILED_INIT = 5002
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




//stores a list of known potential errors, along with information for the end user
namespace ErrorDetail {
	const std::unordered_map<ErrorCode, Error> ErrorManifest = {
		//trace
		//debug
		//info
		{ErrorCode::RENDER_WINDOW_CLOSED,
			{ErrorCode::RENDER_WINDOW_CLOSED, ErrorLevel::INFO,
			 "The render window has closed, exiting.", "RenderSystem"}},
		//warning
		//error
		//fatal
		{ErrorCode::OGRE_NO_AVAILABLE_RENDER_SYSTEM,
			{ErrorCode::OGRE_NO_AVAILABLE_RENDER_SYSTEM, ErrorLevel::FATAL,
			 "No available renders found/accessible", "RenderSystem"}},
		{ErrorCode::SDL_FAILED_INIT,
			{ErrorCode::SDL_FAILED_INIT, ErrorLevel::FATAL,
			 "SDL init failed", "RenderSystem"}}
	};
}