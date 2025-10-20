//Copyright © 2025 Henry Frodsham
#include "RenderSystem.h"
#include <doctest/doctest.h>

TEST_CASE("RenderSystem - singleton behaviour") {
	CHECK_NOTHROW(RenderSystem Obj());
}

TEST_CASE("RenderSystem - new viewport") {
	RenderSystem Obj{};
	CHECK(Obj.CreateViewPort() != NULL);
}

TEST_CASE("RenderSystem - attach model to scene") {

}