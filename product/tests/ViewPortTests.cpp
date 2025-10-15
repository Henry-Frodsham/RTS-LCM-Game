#include "ViewPortController.h"
#include <doctest/doctest.h>

TEST_CASE("ViewPortController - constructor") {
	CHECK_NOTHROW(ViewPortController Obj{});
}

TEST_CASE("ViewPortController - move camera") {
	ViewPortController Obj{};
	Obj.MoveCamera(0.f, 0.f);
	CHECK(Obj.GetCameraAngle() == std::vector<float> {0.f, 0.f});
}

TEST_CASE("ViewPortController - change dimensions") {
	ViewPortController Obj{};
	Obj.ChangeCameraDimensions(0, 0);
	CHECK(Obj.GetCameraDimensions() == std::vector<int> {0, 0});
}
