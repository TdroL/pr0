#pragma once

#include "../rn.hpp"
#include "../src.hpp"
#include "tex.hpp"
#include "types.hpp"
#include "format.hpp"
#include <vector>

namespace rn
{

class Tex2DArray : public Tex
{
public:
	static std::vector<Tex2DArray *> collection;
	static void reloadAll();

	// typedef src::Tex2DArray Source;

	// GLuint id = 0;

	GLsizei width = 0;
	GLsizei height = 0;
	GLsizei size = 0;
	GLint mipLevels = 0;

	GLint internalFormat = format::RGBA8.layout;
	GLenum format = format::RGBA8.components;
	GLenum type = GL_UNSIGNED_BYTE;

	MinFilter minFilter = MIN_LINEAR;
	MagFilter magFilter = MAG_LINEAR;

	Wrap wrapS = WRAP_REPEAT;
	Wrap wrapT = WRAP_REPEAT;

	CompareFunc compareFunc = COMPARE_NONE;

	glm::vec4 borderColor{0.f};

	// std::unique_ptr<Source> source{nullptr};

	std::string texName = "Unnamed 2d texture array";

	Tex2DArray();
	explicit Tex2DArray(std::string &&texName);
	Tex2DArray(GLint internalFormat/*, std::unique_ptr<Source> &&source = nullptr*/);
	Tex2DArray(std::string &&texName, GLint internalFormat/*, std::unique_ptr<Source> &&source = nullptr*/);

	Tex2DArray(Tex2DArray &&rhs);
	Tex2DArray & operator=(Tex2DArray &&rhs);

	Tex2DArray(const Tex2DArray &rhs) = delete;
	Tex2DArray & operator=(const Tex2DArray &rhs) = delete;

	~Tex2DArray();

	GLenum targetType() override
	{
		return GL_TEXTURE_2D_ARRAY;
	}

	void reload();

	void reset();

	bool isDepth() override;
	bool isDepthStencil() override;

	GLsizei bind(GLsizei unit) const override;

	virtual void setParam(GLenum pname, GLfloat param) override;
	virtual void setParam(GLenum pname, GLint param) override;
	virtual void setParam(GLenum pname, const GLfloat *param) override;
	virtual void setParam(GLenum pname, const GLint *param) override;
	virtual void setParamI(GLenum pname, const GLuint *param) override;
	virtual void setParamI(GLenum pname, const GLint *param) override;

	GLenum getAttachmentType() override;
};

} // rn