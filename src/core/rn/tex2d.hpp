#ifndef RN_TEX2D_HPP
#define RN_TEX2D_HPP

#include "../rn.hpp"
#include "../src.hpp"
#include <list>

namespace rn
{

class Tex2D
{
public:
	static std::list<Tex2D *> collection;
	static void reloadAll();

	typedef src::Tex2D Source;

	GLuint id = 0;
	GLsizei width = 0;
	GLsizei height = 0;
	GLint levels = 1;
	GLint filter = GL_LINEAR;
	GLint wrap = GL_CLAMP_TO_EDGE;
	GLint internalFormat = GL_RGBA;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;

	std::unique_ptr<Source> source{nullptr};

	Tex2D();
	Tex2D(GLint internalFormat, std::unique_ptr<Source> &&source = nullptr);
	Tex2D(Tex2D &&rhs);
	Tex2D & operator=(Tex2D &&rhs);

	~Tex2D();

	void reload();

	void reset();

	void bind(GLsizei unit);

	GLenum getAttachmentType();
};

} // rn

#endif