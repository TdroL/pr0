#ifndef GL_MESH_HPP
#define GL_MESH_HPP

#include <string>
#include <memory>
#include <list>
#include <vector>

#include "../gl.hpp"
#include "../src.hpp"
#include "types.hpp"

namespace gl
{

class Mesh
{
public:
	typedef src::Mesh Source;

	static std::list<Mesh *> collection;
	static void reloadAll();
	static void reloadSoftAll();

	GLuint vbo = 0;
	GLuint ibo = 0;
	GLuint vao = 0;

	std::unique_ptr<Source> source{nullptr};
	std::vector<gl::VertexLayout> layouts{};
	std::vector<gl::DrawIndex> indices{};
	std::vector<gl::DrawArray> arrays{};

	std::string meshName = "Unnamed mesh";

	Mesh();
	Mesh(Mesh &&rhs);
	Mesh(std::string &&name);
	~Mesh();

	Mesh(const Mesh &) = delete;

	Mesh & operator=(Mesh &&rhs);

	void load(std::unique_ptr<Source> &&source);
	void reload();
	void reloadSoft();
	void reset();

	void render();
};

} // gl

#endif