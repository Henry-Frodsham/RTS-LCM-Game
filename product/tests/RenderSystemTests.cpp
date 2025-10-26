//Copyright © 2025 Henry Frodsham
#include "RenderSystem.h"
#include <doctest/doctest.h>

TEST_CASE("RenderSystem - singleton behaviour") {
	CHECK_NOTHROW(RenderSystem::GetInstance());
}

TEST_CASE("RenderSystem - new viewport") {
	
	RenderSystem* Obj = &RenderSystem::GetInstance();
	Obj->Init();
	CHECK(Obj->CreateViewPort() != NULL);
	
}

TEST_CASE("RenderSystem - attach model to scene") {

}