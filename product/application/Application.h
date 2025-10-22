//Copyright © 2025 Henry Frodsham
#pragma once
#include "ApplicationStateManager.h"
#include <iostream>

class Application {
public:
	Application() {};
	~Application() {};
	void Start();

private:
	bool Init();

	std::unique_ptr<ApplicationStateManager> StateManager;
};