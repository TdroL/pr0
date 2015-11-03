#include <pch.hpp>

#include "types.hpp"

namespace rn
{

const GLuint LayoutLocation::pos = 0;
const GLuint LayoutLocation::tex = 1;
const GLuint LayoutLocation::norm = 2;

GLint & UniformMeta::set(GLint i)
{
	type = Type::uniform_int;
	this->i = i;
	return this->i;
}

GLuint & UniformMeta::set(GLuint ui)
{
	type = Type::uniform_uint;
	this->ui = ui;
	return this->ui;
}

GLfloat & UniformMeta::set(GLfloat f)
{
	type = Type::uniform_float;
	this->f = f;
	return this->f;
}

glm::vec2 & UniformMeta::set(glm::vec2 v2)
{
	type = Type::uniform_vec2;
	this->v2 = v2;
	return this->v2;
}

glm::vec3 & UniformMeta::set(glm::vec3 v3)
{
	type = Type::uniform_vec3;
	this->v3 = v3;
	return this->v3;
}

glm::vec4 & UniformMeta::set(glm::vec4 v4)
{
	type = Type::uniform_vec4;
	this->v4 = v4;
	return this->v4;
}

glm::mat3 & UniformMeta::set(glm::mat3 m3)
{
	type = Type::uniform_mat3;
	this->m3 = m3;
	return this->m3;
}

glm::mat4 & UniformMeta::set(glm::mat4 m4)
{
	type = Type::uniform_mat4;
	this->m4 = m4;
	return this->m4;
}

GLint * UniformMeta::set(GLint *vi, GLsizei count)
{
	reset();
	type = Type::uniform_v_int;
	this->vi.first = vi;
	this->vi.second = count;

	return this->vi.first;
}

GLuint * UniformMeta::set(GLuint *vui, GLsizei count)
{
	reset();
	type = Type::uniform_v_uint;
	this->vui.first = vui;
	this->vui.second = count;

	return this->vui.first;
}

GLfloat * UniformMeta::set(GLfloat *vf, GLsizei count)
{
	reset();
	type = Type::uniform_v_float;
	this->vf.first = vf;
	this->vf.second = count;

	return this->vf.first;
}

glm::vec2 * UniformMeta::set(glm::vec2 *vv2, GLsizei count)
{
	reset();
	type = Type::uniform_v_vec2;
	this->vv2.first = vv2;
	this->vv2.second = count;

	return this->vv2.first;
}

glm::vec3 * UniformMeta::set(glm::vec3 *vv3, GLsizei count)
{
	reset();
	type = Type::uniform_v_vec3;
	this->vv3.first = vv3;
	this->vv3.second = count;

	return this->vv3.first;
}

glm::vec4 * UniformMeta::set(glm::vec4 *vv4, GLsizei count)
{
	reset();
	type = Type::uniform_v_vec4;
	this->vv4.first = vv4;
	this->vv4.second = count;

	return this->vv4.first;
}

glm::mat3 * UniformMeta::set(glm::mat3 *vm3, GLsizei count)
{
	reset();
	type = Type::uniform_v_mat3;
	this->vm3.first = vm3;
	this->vm3.second = count;

	return this->vm3.first;
}

glm::mat4 * UniformMeta::set(glm::mat4 *vm4, GLsizei count)
{
	reset();
	type = Type::uniform_v_mat4;
	this->vm4.first = vm4;
	this->vm4.second = count;

	return this->vm4.first;
}

void UniformMeta::reset()
{
	switch (type)
	{
		case Type::uniform_v_int:
			if (vi.first)
			{
				delete[] vi.first;
				vi.first = nullptr;
				vi.second = 0;
			}
		break;
		case Type::uniform_v_uint:
			if (vui.first)
			{
				delete[] vui.first;
				vui.first = nullptr;
				vui.second = 0;
			}
		break;
		case Type::uniform_v_float:
			if (vf.first)
			{
				delete[] vf.first;
				vf.first = nullptr;
				vf.second = 0;
			}
		break;
		case Type::uniform_v_vec2:
			if (vv2.first)
			{
				delete[] vv2.first;
				vv2.first = nullptr;
				vv2.second = 0;
			}
		break;
		case Type::uniform_v_vec3:
			if (vv3.first)
			{
				delete[] vv3.first;
				vv3.first = nullptr;
				vv3.second = 0;
			}
		break;
		case Type::uniform_v_vec4:
			if (vv4.first)
			{
				delete[] vv4.first;
				vv4.first = nullptr;
				vv4.second = 0;
			}
		break;
		case Type::uniform_v_mat3:
			if (vm3.first)
			{
				delete[] vm3.first;
				vm3.first = nullptr;
				vm3.second = 0;
			}
		break;
		case Type::uniform_v_mat4:
			if (vm4.first)
			{
				delete[] vm4.first;
				vm4.first = nullptr;
				vm4.second = 0;
			}
		break;
		default:
			// noop
		break;
	}
}

} // rn