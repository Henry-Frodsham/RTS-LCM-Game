//Copyright © 2025 Henry Frodsham
#pragma once
#include "ApplicationStateManager.h"
#include <iostream>
#include <thread>

class Application {
public:
	Application();
	~Application() {};
	void Start();

private:
	bool Init();

	void Loop();

	ApplicationStateManager StateManager;
};