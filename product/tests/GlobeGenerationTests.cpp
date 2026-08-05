// Copyright (c) 2026 Henry Frodsham

#include <doctest/doctest.h>

#include "Globe.h"

TEST_CASE("Generate Globe success") { 
	Globe TestGlobe = Globe();
  TestGlobe.Generate(3, 8352345);
}