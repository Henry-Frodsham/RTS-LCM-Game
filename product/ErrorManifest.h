//Copyright © 2025 Henry Frodsham
#pragma once
#include <string>
#include <array>

// 5 level system, stores how "serious" an error is
enum ErrorLevel : uint16_t {
	TRACE = 0,
	DEBUG = 1,
	INFO = 2,
	WARNING = 3, // unexpected event but not critical
	ERROR = 4, // unexpected event but recoverable
	FATAL = 5 // unexpected event and unrecoverable
};

// codes unique to an error, first digit denotes severity. from range 0000 - 5000
enum ErrorCode : uint32_t {
	//trace
	//debug
	//info
	//warning
	//error
	//fatal
	OGRE_NO_AVAILABLE_RENDER_SYSTEM = 5001,
	SDL_FAILED_INIT = 5002
};

// general information on an error
struct Error {
	ErrorLevel Level;
	ErrorCode Code;
	std::string Message;
	std::string Origin;

	Error(ErrorLevel ELevel, ErrorCode ECode, 
		std::string EMessage, std::string EOrigin)
		: Level(ELevel), Code(ECode), 
		Message(EMessage), Origin(EOrigin){
	}
};




//stores a list of known potential errors, along with information for the end user
namespace ErrorDetail {
	const Error ErrorManifest[] = {
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