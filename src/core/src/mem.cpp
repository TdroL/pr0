#include "mem.hpp"
#include "../gl.hpp"

namespace src
{

using namespace std;

namespace mem
{

Mesh::Mesh(vector<GLfloat> &&vertices, vector<GLuint> &&indices)
{
	setVertices(move(vertices));
	setIndices(move(indices));
}

string Mesh::name()
{
	return sourceName;
}
void Mesh::setName(string &&sourceName)
{
	this->sourceName = move(sourceName);
}

void Mesh::setVertices(vector<GLfloat> &&vertices)
{
	vertexCache = move(vertices);

	vertexData.size = vertexCache.size() * sizeof(vertexCache[0]);
	vertexData.data = vertexCache.data();
	vertexData.usage = GL_STATIC_DRAW;
}

void Mesh::setIndices(vector<GLuint> &&indices, GLenum mode)
{
	indexCache = move(indices);

	if (indexCache.size())
	{
		indexData.size = indexCache.size() * sizeof(indexCache[0]);
		indexData.data = indexCache.data();
		indexData.usage = GL_STATIC_DRAW;

		this->indices.emplace_back(static_cast<GLenum>(mode), static_cast<GLsizeiptr>(indexCache.size()), static_cast<GLenum>(GL_UNSIGNED_INT), static_cast<GLsizeiptr>(0));
	}
}

unique_ptr<src::Mesh> mesh()
{
	return unique_ptr<src::Mesh>{new Mesh{}};
}

unique_ptr<src::Mesh> mesh(vector<GLfloat> &&vertices, vector<GLuint> &&indices)
{
	return unique_ptr<src::Mesh>{new Mesh{move(vertices), move(indices)}};
}

} // mem

} // src