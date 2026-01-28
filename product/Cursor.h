#pragma once
#include "CursorEvent.h"
#include "InputTranslator.h"
#include "EventBus.h"
#include "RenderSystem.h"

class Cursor {

public:
	void ChangeVisibility(ChangeCursorVisibilityEvent Event);

	Cursor(EventBus* ParentBus, InputTranslator* ControlingDevice);

	void Update();
private:
	InputTranslator* DeviceState;

	EventBus* ForeignBus;
};