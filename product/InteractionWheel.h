#pragma once
#include "InputTranslator.h"

//the quick interaction wheel accessible through RT or right click
class InteractionWheel {
public:
	InteractionWheel(InputTranslator* Device);

private:
	InputTranslator* DeviceState;
};