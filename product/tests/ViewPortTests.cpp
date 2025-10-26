//Copyright © 2025 Henry Frodsham
#include "ViewPortController.h"
#include "RenderSystem.h"
#include <doctest/doctest.h>

TEST_CASE("ViewPortController - constructor") {
	CHECK_NOTHROW(ViewPortController Obj{nullptr});
}

TEST_CASE("ViewPortController - move camera") {
	
	RenderSystem* RS = &RenderSystem::GetInstance();
	RS->Init();
	ViewPortController* Obj = RS->CreateViewPort();
	Obj->MoveCamera(0.f, 0.f);
	CHECK(Obj->GetCameraAngle() == std::vector<float> {0.f, 0.f});
	
}

TEST_CASE("ViewPortController - change dimensions") {
	
	RenderSystem* RS = &RenderSystem::GetInstance();
	RS->Init();
	ViewPortController* Obj = RS->CreateViewPort();
	Obj->ChangeCameraDimensions(0, 0);
	CHECK(Obj->GetCameraDimensions() == std::vector<int> {0, 0});
	
}
