#ifndef GL_FBO_HPP
#define GL_FBO_HPP

#include <GL/glew.h>

#include "../util.hpp"
#include "types.hpp"
#include "program.hpp"

#include <list>
#include <vector>
#include <string>
#include <initializer_list>

namespace gl
{

class FBO
{
public:
	enum DepthStencilType
	{
		None,
		RenderBuffer,
		Texture,
	};

	static std::list<FBO *> collection;
	static std::vector<FBO *> activeStack;
	static void reloadAll();
	static void reloadSoftAll();

	static void init();

	GLuint id = 0;
	std::vector<GLuint> colors{};

	unsigned int colorAttachments = 0;
	std::vector<std::string> colorNames{};
	std::vector<gl::TexParams> texParams{};

	GLuint depthStencil = 0;
	DepthStencilType depthStencilType = None;
	gl::TexParams depthStencilParams{
		/* .internalFormat= */ GL_DEPTH24_STENCIL8,
		/* .format= */ GL_DEPTH_STENCIL,
		/* .type= */ GL_UNSIGNED_INT_24_8
	};

	int width = 0;
	int height = 0;

	std::string fboName = "Unnamed FBO";

	FBO();
	explicit FBO(std::string &&name);
	~FBO();

	void setTexParams(size_t id, GLint internalFormat, GLenum format, GLenum type);
	void setDSParams(GLint internalFormat, GLenum format, GLenum type);

	void create(unsigned int colorAttachments = 1, DepthStencilType depthStencilType = RenderBuffer);
	void create(std::vector<std::string> &&colorNames, DepthStencilType depthStencilType = RenderBuffer);
	void reset();
	void resetStorages();

	void reload();
	void reloadSoft();

	GLint getTexturesCount();
	GLint bindTextures(GLint offset = 0);

	void render();
	void render(gl::Program &prog);

	void blit(GLuint target, GLbitfield mask);

	void clear();

	void use();
	void release();
};

class FBOScoper
{
public:
	FBO &fbo;

	FBOScoper(FBO &fbo) : fbo(fbo) { fbo.use(); }
	~FBOScoper() { fbo.release(); }
};

#define GL_FBO_USE(fboSource) gl::FBOScoper UTIL_CONCAT2(FBOScoper, __COUNTER__)(fboSource)

} // gl

#endif