#pragma once
#include "InputDevice.h"

struct RegisterInstanceEvent {
	InputDevice* InstanceDevice;
};

//for events such as an instance needing to control a viewport or another non thread safe request
struct NonThreadSafeRequest {

};