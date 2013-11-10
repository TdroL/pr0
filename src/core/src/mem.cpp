#include "mem.hpp"
#include "../gl.hpp"

namespace src
{

using namespace std;

namespace mem
{

Mesh::Mesh(std::vector<GLfloat> &&vertices, std::vector<GLuint> &&indices)
{
	setVertices(move(vertices));
	setIndices(move(indices));
}

void Mesh::setVertices(std::vector<GLfloat> &&vertices)
{
	vertexCache = move(vertices);

	vertexData.size = vertexCache.size() * sizeof(vertexCache[0]);
	vertexData.data = vertexCache.data();
	vertexData.usage = GL_STATIC_DRAW;
}

void Mesh::setIndices(std::vector<GLuint> &&indices, GLenum mode)
{
	indexCache = move(indices);

	indexData.size = indexCache.size() * sizeof(indexCache[0]);
	indexData.data = indexCache.data();
	indexData.usage = GL_STATIC_DRAW;

	this->indices.emplace_back(static_cast<GLenum>(mode), static_cast<GLsizeiptr>(indexCache.size()), static_cast<GLenum>(GL_UNSIGNED_INT), static_cast<GLsizeiptr>(0));
}

unique_ptr<src::Mesh> mesh()
{
	return unique_ptr<src::Mesh>{new src::mem::Mesh{}};
}

unique_ptr<src::Mesh> mesh(vector<GLfloat> &&vertices, std::vector<GLuint> &&indices)
{
	return unique_ptr<src::Mesh>{new src::mem::Mesh{move(vertices), move(indices)}};
}

} // mem

} // src