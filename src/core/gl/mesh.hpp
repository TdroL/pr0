#ifndef GL_MESH_HPP
#define GL_MESH_HPP

#include <string>
#include <memory>
#include <list>
#include <vector>
#include <GL/glew.h>
#include "types.hpp"
#include "../src.hpp"

namespace gl
{

class Mesh
{
public:
	typedef src::Mesh Source;

	static std::list<Mesh *> collection;
	static void reloadAll();

	GLuint vbo = 0;
	GLuint ibo = 0;
	GLuint vao = 0;

	std::unique_ptr<Source> source{nullptr};
	std::vector<DrawIndex> indices{};
	std::vector<DrawArray> arrays{};

	std::string meshName = "Unnamed mesh";

	Mesh();
	Mesh(std::string &&name);
	~Mesh();

	void load(Source *source);
	void load(std::unique_ptr<Source> &&source);
	void reload();
	void reset();

	void render();
};

} // gl

#endif