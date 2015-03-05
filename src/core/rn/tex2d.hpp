#ifndef RN_TEX2D_HPP
#define RN_TEX2D_HPP

#include "../rn.hpp"
#include "../src.hpp"
#include "types.hpp"
#include "format.hpp"
#include <vector>

namespace rn
{

class Tex2D
{
public:
	static std::vector<Tex2D *> collection;
	static void reloadAll();

	typedef src::Tex2D Source;

	GLuint id = 0;

	GLsizei width = 0;
	GLsizei height = 0;
	GLint levels = 0;

	GLint internalFormat = format::RGBA8.layout;
	GLenum format = format::RGBA8.components;
	GLenum type = GL_UNSIGNED_BYTE;

	MinFilter minFilter = MIN_LINEAR;
	MagFilter magFilter = MAG_LINEAR;

	Wrap wrapS = WRAP_REPEAT;
	Wrap wrapT = WRAP_REPEAT;

	glm::vec4 borderColor{0.f};

	std::unique_ptr<Source> source{nullptr};

	std::string texName = "Unnamed 2d texture";

	Tex2D();
	explicit Tex2D(std::string &&texName);
	Tex2D(GLint internalFormat, std::unique_ptr<Source> &&source = nullptr);
	Tex2D(std::string &&texName, GLint internalFormat, std::unique_ptr<Source> &&source = nullptr);

	Tex2D(Tex2D &&rhs);
	Tex2D & operator=(Tex2D &&rhs);

	Tex2D(const Tex2D &rhs) = delete;
	Tex2D & operator=(const Tex2D &rhs) = delete;

	~Tex2D();

	void reload();

	void reset();

	bool isDepth();
	bool isDepthStencil();

	GLsizei bind(GLsizei unit);

	GLenum getAttachmentType();
};

} // rn

#endif