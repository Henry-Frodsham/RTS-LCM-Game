// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "TransformationMatrix.h"

TEST_CASE("TransformationMatrix - zero delta time always yields a zero "
         "result vector") {
  Eigen::MatrixXf ActionMatrix(2, 2);
  ActionMatrix << 0.0f, 1.0f, 1.0f, -1.0f;
  TransformationMatrix Matrix(ActionMatrix);

  Eigen::VectorXf Source(2);
  Source << 5.0f, 8.0f;

  Matrix.EvaluateAction(Source, 0.0f);

  CHECK(Source[0] == 0.f);
  CHECK(Source[1] == 0.f);
}

TEST_CASE("TransformationMatrix - an identity matrix scales the resource "
         "directly by delta time") {
  Eigen::MatrixXf Identity(2, 2);
  Identity << 1.0f, 0.0f, 0.0f, 1.0f;
  TransformationMatrix Matrix(Identity);

  Eigen::VectorXf Source(2);
  Source << 3.0f, 4.0f;

  Matrix.EvaluateAction(Source, 2.0f);

  CHECK(Source[0] == 6.f);
  CHECK(Source[1] == 8.f);
}

TEST_CASE("TransformationMatrix - a negative delta time reverses the sign "
         "of the resulting change") {
  Eigen::MatrixXf ActionMatrix(2, 2);
  ActionMatrix << 0.0f, 1.0f, 1.0f, -1.0f;
  TransformationMatrix Matrix(ActionMatrix);

  Eigen::VectorXf Source(2);
  Source << 1.0f, 0.0f;

  Matrix.EvaluateAction(Source, -1.0f);

  CHECK(Source[0] == 0.f);
  CHECK(Source[1] == -1.f);
}
