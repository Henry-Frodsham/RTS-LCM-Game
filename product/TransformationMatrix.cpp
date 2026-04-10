// Copyright (c) 2025 Henry Frodsham
#include "TransformationMatrix.h"

TransformationMatrix::TransformationMatrix(Eigen::MatrixXf AMatrix) {
  ActionMatrix = AMatrix;
}

// matrix behaviour is defined as delta = source * action * delta
void TransformationMatrix::EvaluateAction(Eigen::VectorXf& Resource,
                                          float DeltaTime) {
  Resource = DeltaTime * ActionMatrix * Resource;
}
