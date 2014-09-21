#include "mesh.hpp"
#include "../rn.hpp"
#include "../util.hpp"
#include "../ngn.hpp"
#include "../ngn/fs.hpp"
#include <iostream>

namespace rn
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

Mesh::Mesh(Mesh &&rhs)
	: Mesh{}
{
	vbo = rhs.vbo;
	ibo = rhs.ibo;
	vao = rhs.vao;
	source = move(rhs.source);
	layouts = move(rhs.layouts);
	indices = move(rhs.indices);
	arrays = move(rhs.arrays);
	meshName = rhs.meshName; // copy

	rhs.vbo = 0;
	rhs.ibo = 0;
	rhs.vao = 0;

	rhs.source.reset();
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

Mesh & Mesh::operator=(Mesh &&rhs)
{
	reset();

	vbo = rhs.vbo;
	ibo = rhs.ibo;
	vao = rhs.vao;
	source = move(rhs.source);
	layouts = move(rhs.layouts);
	indices = move(rhs.indices);
	arrays = move(rhs.arrays);
	meshName = rhs.meshName; // copy

	rhs.vbo = 0;
	rhs.ibo = 0;
	rhs.vao = 0;

	rhs.source.reset();

	return *this;
}

void Mesh::load(unique_ptr<Source> &&source)
{
	if ( ! rn::status)
	{
		throw string{"rn::Mesh{" + meshName + "}::load() - OpenGL not initialized"};
	}

	reset();

	this->source = move(source);

	reload();
}

void Mesh::reload()
{
	if ( ! source)
	{
		throw string{"rn::Mesh{" + meshName + "}::reload() - empty source"};
	}

	double timer = ngn::time();

	SRC_MESH_USE(*source);

	const auto &vertexData = source->vertexData;
	const auto &indexData = source->indexData;

	layouts = source->layouts;
	indices = source->indices;
	arrays = source->arrays;

	RN_CHECK(glGenBuffers(1, &vbo));
	RN_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	RN_CHECK(glBufferData(GL_ARRAY_BUFFER, vertexData.size, vertexData.data, vertexData.usage));

	if (indexData.size)
	{
		RN_CHECK(glGenBuffers(1, &ibo));
		RN_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
		RN_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size, indexData.data, indexData.usage));
	}

	reloadSoft();

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [Mesh:" << meshName << " {" << source->name() << "}:" << ngn::time() - timer << "s]" << endl;
		// clog << "    layouts=" << layouts.size() << endl;
		// clog << "    indices=" << indices.size() << endl;
		// clog << "    arrays=" << arrays.size() << endl;
	}
}

void Mesh::reloadSoft()
{
	if (vbo)
	{
		RN_CHECK(glGenVertexArrays(1, &vao));
		RN_CHECK(glBindVertexArray(vao));

		for (const auto & layout : layouts)
		{
			RN_CHECK(glEnableVertexAttribArray(layout.index));
			RN_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
			RN_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
			RN_CHECK(glVertexAttribPointer(layout.index, layout.size, layout.type, GL_FALSE, layout.stride, layout.pointer));
		}

		RN_CHECK(glBindVertexArray(0));
	}
}

void Mesh::reset()
{
	source.reset();

	if (vbo)
	{
		RN_CHECK(glDeleteBuffers(1, &vbo));
		vbo = 0;
	}

	if (ibo)
	{
		RN_CHECK(glDeleteBuffers(1, &ibo));
		ibo = 0;
	}

	if (vao)
	{
		RN_CHECK(glDeleteVertexArrays(1, &vao));
		vao = 0;
	}
}

void Mesh::render()
{
	if (vao)
	{
		RN_CHECK(glBindVertexArray(vao));

		for (const auto &index : indices)
		{
			RN_CHECK(glDrawElements(index.mode, index.count, index.type, reinterpret_cast<GLvoid *>(index.offset)));
			rn::stats.triangles += index.count;
		}

		for (const auto &array : arrays)
		{
			RN_CHECK_PARAM(glDrawArrays(array.mode, array.offset, array.count), array.mode << " " << rn::getEnumName(array.mode));
			rn::stats.triangles += array.count;
		}

		RN_CHECK(glBindVertexArray(0));
	}
}

} // rn