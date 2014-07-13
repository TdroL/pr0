#ifndef GL_FBOCUBE_HPP
#define GL_FBOCUBE_HPP

#include <list>
#include <vector>
#include <string>
#include <initializer_list>

#include "../gl.hpp"
#include "../util.hpp"
#include "types.hpp"
#include "program.hpp"

namespace gl
{

class FBOCube
{
public:
	enum DepthStencilType
	{
		None,
		RenderBuffer,
		Texture,
	};

	static std::list<FBOCube *> collection;
	static std::vector<GLuint> activeStack;
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

	std::string fboName = "Unnamed FBOCube";

	FBOCube();
	explicit FBOCube(std::string &&name);
	~FBOCube();

	// void setTexParams(size_t id, GLint internalFormat, GLenum format, GLenum type);
	// void setDSParams(GLint internalFormat, GLenum format, GLenum type);

	// void create(unsigned int colorAttachments = 1, DepthStencilType depthStencilType = RenderBuffer);
	// void create(std::vector<std::string> &&colorNames, DepthStencilType depthStencilType = RenderBuffer);
	// void reset();
	// void resetStorages();

	// void reload();
	// void reloadSoft();

	// void render();
	// void render(gl::Program &prog);

	// void blit(GLuint target, GLbitfield mask);

	// void clear();

	void use();
	void release();
};

class FBOCubeScoper
{
public:
	FBOCube &fbo;

	FBOCubeScoper(FBOCube &fbo) : fbo(fbo) { fbo.use(); }
	~FBOCubeScoper() { fbo.release(); }
};

#define GL_FBOCUBE_USE(fboSource) gl::FBOCubeScoper UTIL_CONCAT2(FBOCubeScoper, __COUNTER__)(fboSource)

} // gl

#endif