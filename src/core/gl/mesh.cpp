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

void Mesh::reloadSoftAll()
{
	for (Mesh *mesh : Mesh::collection)
	{
		try
		{
			mesh->reloadSoft();
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

	const auto &vertexData = source->vertexData;
	const auto &indexData = source->indexData;

	layouts = source->layouts;
	indices = source->indices;
	arrays = source->arrays;

	GL_CHECK(glGenBuffers(1, &vbo));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertexData.size, vertexData.data, vertexData.usage));

	if (indexData.size)
	{
		GL_CHECK(glGenBuffers(1, &ibo));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size, indexData.data, indexData.usage));
	}

	reloadSoft();

	clog << fixed;
	clog << "  [Mesh:" << meshName << " {" << source->name() << "}:" << sys::time() - timer << "s]" << endl;
	// clog << "    layouts=" << layouts.size() << endl;
	// clog << "    indices=" << indices.size() << endl;
	// clog << "    arrays=" << arrays.size() << endl;
}

void Mesh::reloadSoft()
{
	GL_CHECK(glGenVertexArrays(1, &vao));
	GL_CHECK(glBindVertexArray(vao));

	for (const auto & layout : layouts)
	{
		GL_CHECK(glEnableVertexAttribArray(layout.index));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
		GL_CHECK(glVertexAttribPointer(layout.index, layout.size, layout.type, GL_FALSE, layout.stride, layout.pointer));
	}

	GL_CHECK(glBindVertexArray(0));
}

void Mesh::reset()
{
	source.reset();

	if (vbo)
	{
		GL_CHECK(glDeleteBuffers(1, &vbo));
		vbo = 0;
	}

	if (ibo)
	{
		GL_CHECK(glDeleteBuffers(1, &ibo));
		ibo = 0;
	}

	if (vao)
	{
		GL_CHECK(glDeleteVertexArrays(1, &vao));
		vao = 0;
	}
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
		GL_CHECK_PARAM(glDrawArrays(array.mode, array.offset, array.count), array.mode << " " << gl::getEnumName(array.mode));
		gl::stats.triangles += array.count;
	}

	GL_CHECK(glBindVertexArray(0));
}

} // gl