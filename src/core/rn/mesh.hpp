#ifndef RN_MESH_HPP
#define RN_MESH_HPP

#include <string>
#include <memory>
#include <vector>

#include "../rn.hpp"
#include "../src.hpp"
#include "types.hpp"

namespace rn
{

class Mesh
{
public:
	typedef src::Mesh Source;

	static std::vector<Mesh *> collection;
	static Mesh quad;

	static void reloadAll();
	static void reloadSoftAll();

	GLuint vbo = 0;
	GLuint ibo = 0;
	GLuint vao = 0;

	std::unique_ptr<Source> source{nullptr};
	std::vector<rn::VertexLayout> layouts{};
	std::vector<rn::DrawIndex> indices{};
	std::vector<rn::DrawArray> arrays{};

	std::string meshName = "Unnamed mesh";

	GLfloat boundingRadius = 0.f;
	struct BoundingBox{
		glm::vec3 min{};
		glm::vec3 max{};
	} boundingBox{};

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

	void buildBounds(const rn::BufferData &vertexData, const rn::VertexLayout &layout);

	void render();
};

} // rn

#endif