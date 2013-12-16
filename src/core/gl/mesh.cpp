#include "mesh.hpp"
#include "../gl.hpp"
#include "../sys.hpp"
#include "../sys/fs.hpp"
#include <iostream>

namespace gl
{
using namespace std;

list<Mesh *> Mesh::collection{};

void Mesh::reloadAll()
{
	for (Mesh *mesh : Mesh::collection)
	{
		try
		{
			mesh->reload();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

Mesh::Mesh()
{
	Mesh::collection.push_back(this);
}

Mesh::Mesh(string &&name)
	: Mesh{}
{
	meshName = move(name);
}

Mesh::~Mesh()
{
	Mesh::collection.remove(this);
}

void Mesh::load(Source *source)
{
	load(unique_ptr<Source>{source});
}

void Mesh::load(unique_ptr<Source> &&source)
{
	if ( ! gl::status)
	{
		throw string{"gl::Mesh{" + meshName + "}::load() - OpenGL not initialized"};
	}

	reset();

	this->source = move(source);

	reload();
}

void Mesh::reload()
{
	if ( ! source)
	{
		throw string{"gl::Mesh{" + meshName + "}::reload() - empty source"};
	}

	double timer = sys::time();

	SRC_MESH_USE(*source);

	const auto &layouts = source->layouts;
	const auto &vertexData = source->vertexData;
	const auto &indexData = source->indexData;

	indices = source->indices;
	arrays = source->arrays;

	GL_CHECK(glGenVertexArrays(1, &vao));
	GL_CHECK(glBindVertexArray(vao));

	GL_CHECK(glGenBuffers(1, &vbo));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertexData.size, vertexData.data, vertexData.usage));

	if (indexData.size)
	{
		GL_CHECK(glGenBuffers(1, &ibo));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size, indexData.data, indexData.usage));
	}

	for (const auto & layout : layouts)
	{
		GL_CHECK(glEnableVertexAttribArray(layout.index));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
		GL_CHECK(glVertexAttribPointer(layout.index, layout.size, layout.type, GL_FALSE, layout.stride, layout.pointer));
	}

	GL_CHECK(glBindVertexArray(0));

	clog << fixed;
	clog << "  [" << meshName << "{" << source->name() << "}:" << sys::time() - timer << "s]" << endl;
}

void Mesh::reset()
{
	source.reset();

	GL_CHECK(glDeleteBuffers(1, &vbo));
	GL_CHECK(glDeleteBuffers(1, &ibo));
	GL_CHECK(glDeleteVertexArrays(1, &vao));

	vbo = ibo = vao = 0;
}

void Mesh::render()
{
	GL_CHECK(glBindVertexArray(vao));

	for (const auto &index : indices)
	{
		GL_CHECK(glDrawElements(index.mode, index.count, index.type, reinterpret_cast<GLvoid *>(index.offset)));
		gl::stats.triangles += index.count;
	}

	for (const auto &array : arrays)
	{
		GL_CHECK(glDrawArrays(array.mode, array.offset, array.count));
		gl::stats.triangles += array.count;
	}

	GL_CHECK(glBindVertexArray(0));
}

} // gl