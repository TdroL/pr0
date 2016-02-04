#pragma once

#include "../rn.hpp"
#include "types.hpp"

namespace rn
{

class Tex
{
public:
	GLuint id = 0;

	virtual ~Tex() {}

	virtual GLenum targetType() = 0;

	virtual bool isDepth() = 0;
	virtual bool isDepthStencil() = 0;

	virtual GLsizei bind(GLsizei unit) const = 0;

	virtual void setParam(GLenum pname, GLfloat param) = 0;
	virtual void setParam(GLenum pname, GLint param) = 0;
	virtual void setParam(GLenum pname, const GLfloat *param) = 0;
	virtual void setParam(GLenum pname, const GLint *param) = 0;
	virtual void setParamI(GLenum pname, const GLuint *param) = 0;
	virtual void setParamI(GLenum pname, const GLint *param) = 0;

	virtual GLenum getAttachmentType() = 0;
};

} // rn