#ifndef RN_TYPES_HPP
#define RN_TYPES_HPP

#include "../rn.hpp"
#include <vector>
#include <iostream>

namespace rn
{

struct LayoutLocation
{
	static const GLuint pos;
	static const GLuint tex;
	static const GLuint norm;
};

struct VertexLayout
{
	GLuint index = 0;
	GLint size = 0;
	GLenum type = GL_FLOAT;
	GLsizei stride = 0;
	GLuint offset = 0;

	VertexLayout() = default;
	VertexLayout(GLuint index, GLint size, GLenum type, GLsizei stride, GLuint offset)
		: index{index}, size{size}, type{type}, stride{stride}, offset{offset}
	{
		// std::cout << "layout: " << index << ", " << size << ", " << rn::getEnumName(type) << ", " << stride << ", " << offset << std::endl;
	}

	// VertexLayout(int index, int size, int type, int stride, int offset)
	// 	: index{static_cast<GLuint>(index)}, size{static_cast<GLint>(size)}, type{static_cast<GLenum>(type)}, stride{static_cast<GLsizei>(stride)}, offset{static_cast<GLuint>(offset)}
	// {}

	VertexLayout(const VertexLayout &) = default;
	VertexLayout(VertexLayout &&) = default;

	VertexLayout & operator=(const VertexLayout &) & = default;
	VertexLayout & operator=(VertexLayout &&) & = default;

};

struct BufferData
{
	GLsizeiptr size = 0;
	GLvoid *data = nullptr;
	GLenum usage = GL_NONE;

	BufferData() = default;
	BufferData(GLsizeiptr size, GLvoid *data, GLenum usage)
		: size{size}, data{data}, usage{usage}
	{}

	BufferData(int size, int data, int usage)
		: size{static_cast<GLsizeiptr>(size)}, data{reinterpret_cast<GLvoid *>(data)}, usage{static_cast<GLenum>(usage)}
	{}

	BufferData(const BufferData &) = default;
	BufferData(BufferData &&) = default;

	BufferData & operator=(const BufferData &) & = default;
	BufferData & operator=(BufferData &&) & = default;
};

struct DrawIndex
{
	GLenum mode = GL_NONE;
	GLsizeiptr count = 0;
	GLenum type = GL_NONE;
	GLsizeiptr offset = 0;

	DrawIndex() = default;
	DrawIndex(GLenum mode, GLsizeiptr count, GLenum type, GLsizeiptr offset)
		: mode{mode}, count{count}, type{type}, offset{offset}
	{}

	DrawIndex(int mode, int count, int type, int offset)
		: mode{static_cast<GLenum>(mode)}, count{static_cast<GLsizeiptr>(count)}, type{static_cast<GLenum>(type)}, offset{static_cast<GLsizeiptr>(offset)}
	{}

	DrawIndex(const DrawIndex &) = default;
	DrawIndex(DrawIndex &&) = default;

	DrawIndex & operator=(const DrawIndex &) & = default;
	DrawIndex & operator=(DrawIndex &&) & = default;
};

struct DrawArray
{
	GLenum mode = GL_NONE;
	GLsizeiptr offset = 0;
	GLsizeiptr count = 0;

	DrawArray() = default;

	DrawArray(GLenum mode, GLsizeiptr offset, GLsizeiptr count)
		: mode{mode}, offset{offset}, count{count}
	{}

	DrawArray(int mode, int offset, int count)
		: mode{static_cast<GLenum>(mode)}, offset{static_cast<GLsizeiptr>(offset)}, count{static_cast<GLsizeiptr>(count)}
	{}

	DrawArray(const DrawArray &) = default;
	DrawArray(DrawArray &&) = default;

	DrawArray & operator=(const DrawArray &) & = default;
	DrawArray & operator=(DrawArray &&) & = default;
};

struct UniformMeta
{
	enum Type
	{
		uniform_none,
		uniform_int,
		uniform_uint,
		uniform_float,
		uniform_vec2,
		uniform_vec3,
		uniform_vec4,
		uniform_mat3,
		uniform_mat4,
		uniform_v_int,
		uniform_v_uint,
		uniform_v_float,
		uniform_v_vec2,
		uniform_v_vec3,
		uniform_v_vec4,
		uniform_v_mat3,
		uniform_v_mat4,
	};

	UniformMeta::Type type;
	GLint location;

	union
	{
		GLint i;
		GLuint ui;
		GLfloat f;
		glm::vec2 v2;
		glm::vec3 v3;
		glm::vec4 v4;
		glm::mat3 m3;
		glm::mat4 m4;
		std::pair<GLint *, GLsizei> vi;
		std::pair<GLuint *, GLsizei> vui;
		std::pair<GLfloat *, GLsizei> vf;
		std::pair<glm::vec2 *, GLsizei> vv2;
		std::pair<glm::vec3 *, GLsizei> vv3;
		std::pair<glm::vec4 *, GLsizei> vv4;
		std::pair<glm::mat3 *, GLsizei> vm3;
		std::pair<glm::mat4 *, GLsizei> vm4;
	};

	UniformMeta() : type{uniform_none}, location{-1}, m4{0} {}

	GLint & set(GLint i);
	GLuint & set(GLuint ui);
	GLfloat & set(GLfloat f);
	glm::vec2 & set(glm::vec2 v2);
	glm::vec3 & set(glm::vec3 v3);
	glm::vec4 & set(glm::vec4 v4);
	glm::mat3 & set(glm::mat3 m3);
	glm::mat4 & set(glm::mat4 m4);
	GLint * set(GLint *vi, GLsizei count);
	GLuint * set(GLuint *vui, GLsizei count);
	GLfloat * set(GLfloat *vf, GLsizei count);
	glm::vec2 * set(glm::vec2 *vv2, GLsizei count);
	glm::vec3 * set(glm::vec3 *vv3, GLsizei count);
	glm::vec4 * set(glm::vec4 *vv4, GLsizei count);
	glm::mat3 * set(glm::mat3 *vm3, GLsizei count);
	glm::mat4 * set(glm::mat4 *vm4, GLsizei count);

	void reset();

	~UniformMeta()
	{
		reset();
	}
};

struct BufferMeta
{
	GLuint index;
	GLuint bufferId;

	BufferMeta() : index{TypeLimit<GLuint>::max}, bufferId{TypeLimit<GLuint>::max} {}
};

struct TexParams
{
	GLint internalFormat = GL_RGBA;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;

	TexParams() = default;

	TexParams(GLint internalFormat, GLenum format, GLenum type)
		: internalFormat{internalFormat}, format{format}, type{type}
	{}
};

enum MinFilter
{
	MIN_NEAREST         = GL_NEAREST,
	MIN_LINEAR          = GL_LINEAR,
	MIN_NEAREST_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
	MIN_LINEAR_NEAREST  = GL_LINEAR_MIPMAP_NEAREST,
	MIN_NEAREST_LINEAR  = GL_NEAREST_MIPMAP_LINEAR,
	MIN_LINEAR_LINEAR   = GL_LINEAR_MIPMAP_LINEAR
};

enum MagFilter
{
	MAG_NEAREST = GL_NEAREST,
	MAG_LINEAR  = GL_LINEAR
};

enum Wrap
{
	WRAP_CLAMP  = GL_CLAMP_TO_EDGE,
	WRAP_MIRROR = GL_MIRRORED_REPEAT,
	WRAP_REPEAT = GL_REPEAT,
	WRAP_BORDER = GL_CLAMP_TO_BORDER
};

enum CompareFunc {
	COMPARE_NONE     = GL_NONE,
	COMPARE_LEQUAL   = GL_LEQUAL,
	COMPARE_GEQUAL   = GL_GEQUAL,
	COMPARE_LESS     = GL_LESS,
	COMPARE_GREATER  = GL_GREATER,
	COMPARE_EQUAL    = GL_EQUAL,
	COMPARE_NOTEQUAL = GL_NOTEQUAL,
	COMPARE_ALWAYS   = GL_ALWAYS,
	COMPARE_NEVER    = GL_NEVER
};

static const GLenum BUFFER_DEPTH   = GL_DEPTH_BUFFER_BIT;
static const GLenum BUFFER_STENCIL = GL_STENCIL_BUFFER_BIT;
static const GLenum BUFFER_COLOR   = GL_COLOR_BUFFER_BIT;

typedef GLbitfield BuffersMask;

} // rn

#endif