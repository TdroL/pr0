#ifndef SRC_MEM_HPP
#define SRC_MEM_HPP

#include "../src.hpp"
#include "../gl/types.hpp"

namespace src
{

namespace mem
{

class Mesh : public src::Mesh
{
public:
	std::vector<GLfloat> vertexCache{};
	std::vector<GLuint> indexCache{};

	std::string sourceName = "Unnamed memory mesh source";

	Mesh() = default;
	Mesh(std::vector<GLfloat> &&vertices, std::vector<GLuint> &&indices = {});

	std::string name() override;
	void setName(std::string &&sourceName);

	void setVertices(std::vector<GLfloat> &&vertices);
	void setIndices(std::vector<GLuint> &&indices, GLenum mode = GL_TRIANGLES);
};

std::unique_ptr<src::Mesh> mesh();
std::unique_ptr<src::Mesh> mesh(std::vector<GLfloat> &&vertices, std::vector<GLuint> &&indices = {});

} // mem

} // src

#endif