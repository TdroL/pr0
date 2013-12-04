#ifndef GL_FBO_HPP
#define GL_FBO_HPP

#include <GL/glew.h>
#include <list>

namespace gl
{

class FBO
{
public:
	static std::list<FBO *> collection;
	static void reloadAll();

	GLuint id = 0;

	int width = 0;
	int height = 0;

	FBO();
	~FBO();

	void reload();
};

} // gl

#endif