#pragma once

#include "../rn.hpp"

namespace rn
{

namespace math
{

glm::mat4 infRevPerspectiveMatrix(float fovy, float aspect, float zNear);

void infRevPerspectiveMatrixAndInverse(glm::mat4 &matrix, glm::mat4 &invMatrix, float fovy, float aspect, float zNear);

glm::mat4 orthoLHMatrix(float left, float right, float bottom, float top, float zNear, float zFar);

} // math

} // rn