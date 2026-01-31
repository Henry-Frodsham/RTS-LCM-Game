// Copyright © 2025 Henry Frodsham
#pragma once

// notification event to change the perceived size of a viewport
struct ResizedViewPortEvent {
	float X, Y;
	ResizedViewPortEvent(float x, float y) : X(x), Y(y){

	}
};