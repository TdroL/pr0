#ifndef RN_RENDERBUFFER_HPP
#define RN_RENDERBUFFER_HPP

#include "../rn.hpp"

namespace rn
{

class Renderbuffer
{
public:
	GLuint id = 0;
	GLenum internalFormat = GL_DEPTH24_STENCIL8;

	GLsizei width = 0;
	GLsizei height = 0;

	Renderbuffer() = default;
	explicit Renderbuffer(GLenum internalFormat);
	Renderbuffer(GLenum internalFormat, GLsizei width, GLsizei height);

	Renderbuffer(Renderbuffer &&rhs);
	Renderbuffer & operator=(Renderbuffer &&rhs);

	~Renderbuffer();

	void reload();
	void reset();

	GLenum getAttachmentType();
};

} // rn

#endif