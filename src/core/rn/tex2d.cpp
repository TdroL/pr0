#include "tex2d.hpp"

#include "../rn.hpp"

namespace rn
{

Tex2D::Tex2D(GLint internalFormat, GLenum format, GLenum type)
	: internalFormat{internalFormat}, format{format}, type{type}
{}

Tex2D::Tex2D(Tex2D &&rhs)
	: id{rhs.id}, internalFormat{rhs.internalFormat}, format{rhs.format}, type{rhs.type}
{
	rhs.id = 0;
	rhs.reset();
}

Tex2D::~Tex2D()
{
	reset();
}

Tex2D & Tex2D::operator=(Tex2D &&rhs)
{
	reset();

	id = rhs.id;
	internalFormat = rhs.internalFormat;
	format = rhs.format;
	type = rhs.type;

	rhs.id = 0;
	rhs.reset();

	return *this;
}

void Tex2D::create(GLsizei width, GLsizei height, GLint filter, GLint wrap)
{
	reset();

	RN_CHECK(glGenTextures(1, &id));

	RN_CHECK(glBindTexture(GL_TEXTURE_2D, id));

	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));

	RN_CHECK_PARAM(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr), rn::getEnumName(internalFormat) << ", " << rn::getEnumName(format) << ", " << rn::getEnumName(type));

	RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

void Tex2D::reset()
{
	if (id)
	{
		RN_CHECK(glDeleteTextures(1, &id));
		id = 0;
	}
}

void Tex2D::bind(GLsizei unit)
{
	RN_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
	RN_CHECK(glBindTexture(GL_TEXTURE_2D, id));
}

GLenum Tex2D::getAttachmentType()
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