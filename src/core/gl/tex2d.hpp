#ifndef GL_TEX2D_HPP
#define GL_TEX2D_HPP

#include "../gl.hpp"

namespace gl
{

class Tex2D
{
public:
	GLuint id = 0;
	GLint internalFormat = GL_RGBA;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;

	Tex2D() = default;
	Tex2D(GLint internalFormat, GLenum format, GLenum type);
	Tex2D(Tex2D &&rhs);
	Tex2D & operator=(Tex2D &&rhs);

	~Tex2D();

	void create(GLsizei width, GLsizei height, GLint filter = GL_LINEAR, GLint wrap = GL_CLAMP_TO_EDGE);
	void reset();

	void bind(GLsizei unit);

	GLenum getAttachmentType();
};

} // gl

#endif