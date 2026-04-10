// Copyright (c) 2025 Henry Frodsham
#include <doctest/doctest.h>

#include "TransformationMatrix.h"

TEST_CASE("TransformationMatrix - evaluation formula identity check") {
  Eigen::MatrixXf TestActionMatrix(2, 2);
  TestActionMatrix << 0.0f, 1.0f, 1.0f, -1.0f;

  TransformationMatrix TestTransformationMatrix =
      TransformationMatrix(TestActionMatrix);

  Eigen::VectorXf TestSource(2);
  TestSource << 1.0f, 0.0f;

  float TestDeltaTime = 1.0f;

  Eigen::VectorXf TestExpected(2);
  TestExpected << 0.0f, 1.0f;

  TestTransformationMatrix.EvaluateAction(TestSource, TestDeltaTime);
  CHECK(TestSource == TestExpected);
}

TEST_CASE("TransformationMatrix - evaluate negative float resource exchange") {
  Eigen::MatrixXf TestActionMatrix(2, 2);
  TestActionMatrix << 0.0f, 1.0f, 1.0f, -1.0f;

  TransformationMatrix TestTransformationMatrix =
      TransformationMatrix(TestActionMatrix);

  Eigen::VectorXf TestSource(2);
  TestSource << 0.0f, 1.0f;

  float TestDeltaTime = 1.0f;

  Eigen::VectorXf TestExpected(2);
  TestExpected << 1.0f, -1.0f;

  TestTransformationMatrix.EvaluateAction(TestSource, TestDeltaTime);
  CHECK(TestSource == TestExpected);
}

TEST_CASE("TransformationMatrix - non trivial value resource evaluation") {
  Eigen::MatrixXf TestActionMatrix(2, 2);
  TestActionMatrix << 0.0f, 1.0f, 1.0f, -1.0f;

  TransformationMatrix TestTransformationMatrix =
      TransformationMatrix(TestActionMatrix);

  Eigen::VectorXf TestSource(2);
  TestSource << 2.0f, 3.0f;

  float TestDeltaTime = 1.0f;

  Eigen::VectorXf TestExpected(2);
  TestExpected << 3.0f, -1.0f;

  TestTransformationMatrix.EvaluateAction(TestSource, TestDeltaTime);
  CHECK(TestSource == TestExpected);
}

TEST_CASE("TransformationMatrix - evaluation with time scaling") {
  Eigen::MatrixXf TestActionMatrix(2, 2);
  TestActionMatrix << 0.0f, 1.0f, 1.0f, -1.0f;

  TransformationMatrix TestTransformationMatrix =
      TransformationMatrix(TestActionMatrix);

  Eigen::VectorXf TestSource(2);
  TestSource << 4.0f, 2.0f;

  float TestDeltaTime = 0.5f;

  Eigen::VectorXf TestExpected(2);
  TestExpected << 1.0f, 1.0f;

  TestTransformationMatrix.EvaluateAction(TestSource, TestDeltaTime);
  CHECK(TestSource == TestExpected);
}
