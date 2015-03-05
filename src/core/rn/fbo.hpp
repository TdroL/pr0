#ifndef RN_FBO_HPP
#define RN_FBO_HPP

#include <vector>
#include <string>
#include <initializer_list>

#include "../rn.hpp"
#include "../util.hpp"
#include "types.hpp"
#include "mesh.hpp"
#include "program.hpp"

namespace rn
{

class FBO
{
public:
	enum DepthType
	{
		Renderbuffer,
		Texture,
		None
	};

	struct TexContainer
	{
		GLuint id = 0;
		GLsizei levels = 1;
		GLint layer = 0;
		GLint internalFormat = GL_RGBA8;
		GLint minFilter = GL_LINEAR;
		GLint magFilter = GL_LINEAR;
		bool enabled = false;
		bool owning = true;
	};

	struct DepthContainer
	{
		GLuint id = 0;
		GLint internalFormat = GL_DEPTH24_STENCIL8;
		DepthType type{None};
	};

	static std::vector<FBO *> collection;
	static std::vector<FBO *> activeStack;
	static rn::Mesh mesh;
	static rn::Program prog;
	static void reloadAll();
	static void reloadSoftAll();

	static void init();

	GLuint id = 0;

	std::vector<TexContainer> colors{};
	DepthContainer depth{};

	glm::vec4 clearColor{0.f, 0.f, 0.f, 0.f};

	GLint width = 0;
	GLint height = 0;

	std::string fboName = "Unnamed FBO";

	FBO();
	explicit FBO(std::string &&name);
	~FBO();

	void setColorTex(size_t i, GLint internalFormat, GLsizei levels = 1, GLint minFilter = GL_LINEAR, GLint magFilter = GL_LINEAR);
	void setDepthTex(GLint internalFormat);
	void setDepthBuf(GLint internalFormat);

	void attachColorTex(size_t i, GLuint id, GLint layer = 0);

	void clone(const FBO &fbo);

	void create();
	void reset();
	void resetDepthStorage();
	void resetColorStorages();

	void reload();
	void reloadSoft();

	GLsizei bindColorTex(GLsizei unit, size_t i);
	GLsizei bindDepthTex(GLsizei unit);

	void blit(GLuint target, GLbitfield mask, GLint filter = GL_NEAREST, GLsizei targetWidth = 0, GLsizei targetHeight = 0);
	void blit(FBO &fbo, GLbitfield mask, GLint filter = GL_NEAREST);

	void clear();
	void clear(GLbitfield mask);

	void use();
	void forgo();
};

class FBOScoper
{
public:
	FBO &fbo;

	FBOScoper(FBO &fbo) : fbo(fbo) { fbo.use(); }
	~FBOScoper() { fbo.forgo(); }
};

#define RN_FBO_USE(fboSource) rn::FBOScoper UTIL_CONCAT2(FBOScoper, __COUNTER__)(fboSource)

} // rn

#endif