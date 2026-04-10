// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <Eigen/Dense>

class TransformationMatrix {
 public:
  explicit TransformationMatrix(Eigen::MatrixXf AMatrix);

  // evaluate the resource exchange between a resource and the action matri
  void EvaluateAction(Eigen::VectorXf& Resource, float DeltaTime);

 private:
  Eigen::MatrixXf ActionMatrix;
};
