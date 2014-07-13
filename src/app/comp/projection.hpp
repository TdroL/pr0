#ifndef APP_COMP_PROJECTION_HPP
#define APP_COMP_PROJECTION_HPP

#include <core/ecs/component.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct Projection : public ecs::Component<Projection>
{

	float fovy = 45.f;
	float aspect = 16.f / 9.f;
	float zNear = 1.f / 128.f;
	float zFar = 1.f * 128.f;

	bool dirty = false;

	glm::mat4 matrix{1.f};

	Projection()
	{
		createMatrix();
	}

	void setFovy(float fovy)
	{
		this->fovy = fovy;
		dirty = true;
	}

	void setAspect(float aspect)
	{
		this->aspect = aspect;
		dirty = true;
	}

	void setNear(float zNear)
	{
		this->zNear = zNear;
		dirty = true;
	}

	void setFar(float zFar)
	{
		this->zFar = zFar;
		dirty = true;
	}

	void createMatrix()
	{
		matrix = glm::perspective(fovy, aspect, zNear, zFar);
		dirty = false;
	}

	const glm::mat4 & getMatrix()
	{
		if (dirty)
		{
			createMatrix();
		}

		return matrix;
	}

	glm::mat4 getInverseMatrix()
	{
		return glm::inverse(getMatrix());
	}
};

} // comp

#endif