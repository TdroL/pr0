#ifndef SRC_MEM_HPP
#define SRC_MEM_HPP

#include "../src.hpp"
#include "../rn/types.hpp"
#include <array>
#include <vector>
#include <initializer_list>

namespace src
{

namespace mem
{

/**
 * Mesh
 */
class Mesh : public src::Mesh
{
public:
	std::vector<GLfloat> vertexCache{};
	std::vector<GLuint> indexCache{};

	std::string sourceName = "Unnamed in-memory mesh source";

	Mesh() = default;
	Mesh(std::vector<GLfloat> &&vertices, std::vector<GLuint> &&indices = {});

	std::string name() override;
	void setName(std::string &&sourceName);

	void setVertices(std::vector<GLfloat> &&vertices);
	void setIndices(std::vector<GLuint> &&indices, GLenum mode = GL_TRIANGLES);
};

std::unique_ptr<src::Mesh> mesh();
std::unique_ptr<src::Mesh> mesh(std::vector<GLfloat> &&vertices, std::vector<GLuint> &&indices = {});

/**
 * Tex2D
 */
class Tex2D : public src::Tex2D
{
public:
	std::string sourceName = "Unnamed in-memory 2D texture source";

	Tex2D() = default;
	Tex2D(GLsizei width, GLsizei height, std::unique_ptr<GLubyte[]> &&data = nullptr, std::size_t size = 0, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE, GLint levels = 1, GLint alignment = 1);

	std::string name() override;
	void setName(std::string &&sourceName);
};

std::unique_ptr<src::Tex2D> tex2d(GLsizei width, GLsizei height, std::unique_ptr<GLubyte[]> &&data = nullptr, std::size_t size = 0, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE, GLint levels = 1, GLint alignment = 1);
} // mem

} // src

#endif