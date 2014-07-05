#ifndef GL_RENDERBUFFER_HPP
#define GL_RENDERBUFFER_HPP

#include "../gl.hpp"

namespace gl
{

class Renderbuffer
{
public:
	GLuint id = 0;
	GLenum internalFormat = GL_DEPTH24_STENCIL8;

	Renderbuffer() = default;
	explicit Renderbuffer(GLenum internalFormat);
	Renderbuffer(Renderbuffer &&rhs);
	Renderbuffer & operator=(Renderbuffer &&rhs);

	~Renderbuffer();

	void create(GLsizei width, GLsizei height);
	void reset();

	GLenum getAttachmentType();
};

} // gl

#endif