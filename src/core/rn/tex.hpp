#ifndef RN_TEX_HPP
#define RN_TEX_HPP

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

	virtual GLsizei bind(GLsizei unit) = 0;

	virtual GLenum getAttachmentType() = 0;
};

} // rn

#endif