#include "math.hpp"

namespace rn
{

namespace math
{

glm::mat4 infRevPerspectiveMatrix(float fovy, float aspect, float zNear)
{
	float const tanHalfFovy = glm::tan(fovy * 0.5f);

	glm::mat4 matrix{0.f};
	matrix[0][0] = 1.f / (aspect * tanHalfFovy);
	matrix[1][1] = 1.f / tanHalfFovy;
	matrix[2][3] = 1.f;
	matrix[3][2] = zNear;

	return matrix;
}

void infRevPerspectiveMatrixAndInverse(glm::mat4 &matrix, glm::mat4 &invMatrix, float fovy, float aspect, float zNear)
{
	float const tanHalfFovy = glm::tan(fovy * 0.5f);

	invMatrix = glm::mat4{0.f};
	invMatrix[0][0] = aspect * tanHalfFovy;
	invMatrix[1][1] = tanHalfFovy;
	invMatrix[2][3] = 1.f / zNear;
	invMatrix[3][2] = 1.f;

	matrix = glm::mat4{0.f};
	matrix[0][0] = 1.f / invMatrix[0][0];
	matrix[1][1] = 1.f / invMatrix[1][1];
	matrix[2][3] = 1.f;
	matrix[3][2] = zNear;
}

glm::mat4 orthoLHMatrix(float left, float right, float bottom, float top, float zNear, float zFar)
{
	// 2/(r-l)      0            0           (l+r)/(l-r)
	// 0            2/(t-b)      0           (t+b)/(b-t)
	// 0            0            1/(zn-zf)   zn/(zn-zf)
	// 0            0            0           1
	glm::mat4 matrix{1.f};
	matrix[0][0] = 2.f / (right - left);
	matrix[1][1] = 2.f / (top - bottom);
	matrix[2][2] = 1.f / (zFar - zNear);
	matrix[3][0] = (left + right) / (left - right);
	matrix[3][1] = (bottom + top) / (bottom - top);
	matrix[3][2] = zNear / (zNear - zFar);

	return matrix;
}

} // math

} // rn