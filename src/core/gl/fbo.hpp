#ifndef GL_FBO_HPP
#define GL_FBO_HPP

#include <GL/glew.h>

#include "../util.hpp"

#include <list>
#include <vector>
#include <string>
#include <initializer_list>

namespace gl
{

class FBO
{
public:
	static std::list<FBO *> collection;
	static void reloadAll();
	static void reloadSoftAll();

	static void init();

	GLuint id = 0;
	std::vector<GLuint> colors{};
	GLuint renderbuffer = 0;

	unsigned int colorAttachments = 1;
	bool depthAttachment = true;
	bool stencilAttachment = false;

	int width = 0;
	int height = 0;

	std::string fboName = "Unnamed FBO";

	FBO();
	explicit FBO(std::string &&name);
	~FBO();

	void create(unsigned int colorAttachments = 1, bool depthAttachment = true, bool stelcilAttachment = false);
	void reset();

	void reload();
	void reloadSoft();

	void render();

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