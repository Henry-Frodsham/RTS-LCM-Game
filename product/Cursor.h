#pragma once
#include "CursorEvent.h"
#include "InputTranslator.h"
#include "EventBus.h"
class Cursor {

public:
	void ChangeVisibility(ChangeCursorVisibilityEvent Event);

	Cursor(EventBus* ParentBus);
private:
	InputTranslator* DeviceState;

	EventBus* ForeignBus;
};