//Copyright © 2025 Henry Frodsham
#pragma once
#include <string>
#include <array>

// 5 level system
const enum ErrorLevel : uint16_t {
	TRACE = 0,
	DEBUG = 1,
	INFO = 2,
	WARNING = 3, // unexpected event but not critical
	ERROR = 4, // unexpected event but recoverable
	FATAL = 5 // unexpected event and unrecoverable
};

const struct Error {
	ErrorLevel Level;
	ErrorCode Code;
	std::string Message;
	std::string Origin;
};

// 0000 - 5000
const enum class ErrorCode : uint32_t {
	//trace
	//debug
	//info
	//warning
	//error
	//fatal
	OGRE_NO_AVAILABLE_RENDER_SYSTEM = 50001
};


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
	};
}