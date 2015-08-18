#include <pch.hpp>

#include "renderbuffer.hpp"

#include "../rn.hpp"

namespace rn
{

Renderbuffer::Renderbuffer(GLenum internalFormat)
	: internalFormat{internalFormat}
{}

Renderbuffer::Renderbuffer(GLenum internalFormat, GLsizei width, GLsizei height)
	: internalFormat{internalFormat}, width{width}, height{height}
{}

Renderbuffer::Renderbuffer(Renderbuffer &&rhs)
	: id{rhs.id}, internalFormat{rhs.internalFormat}, width{rhs.width}, height{rhs.height}
{
	rhs.id = 0;
	rhs.reset();
}

Renderbuffer::~Renderbuffer()
{
	reset();
}

Renderbuffer & Renderbuffer::operator=(Renderbuffer &&rhs)
{
	reset();

	id = rhs.id;
	internalFormat = rhs.internalFormat;
	width = rhs.width;
	height = rhs.height;

	rhs.id = 0;
	rhs.reset();

	return *this;
}

void Renderbuffer::reload()
{
	reset();

	RN_CHECK(glGenRenderbuffers(1, &id));

	RN_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, id));

	RN_CHECK_PARAM(glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height), rn::getEnumName(internalFormat));

	RN_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

void Renderbuffer::reset()
{
	if (id)
	{
		RN_CHECK(glDeleteRenderbuffers(1, &id));
		id = 0;
	}
}

GLenum Renderbuffer::getAttachmentType()
{
	switch (internalFormat)
	{
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32F:
		{
			return GL_DEPTH_ATTACHMENT;
		}
		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH32F_STENCIL8:
		{
			return GL_DEPTH_STENCIL_ATTACHMENT;
		}
		default:
		{
			return GL_COLOR_ATTACHMENT0;
		}
	}
}

}