#include "mesh.hpp"
#include "../rn.hpp"
#include "../util.hpp"
#include "../ngn.hpp"
#include "../ngn/fs.hpp"
#include "../src/mem.hpp"
#include <iostream>
#include <algorithm>
#include <limits>

namespace rn
{
using namespace std;

vector<Mesh *> Mesh::collection{};

Mesh Mesh::quad{"Mesh quad"};

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

	boundingRadius = rhs.boundingRadius;
	boundingBox.min = rhs.boundingBox.min;
	boundingBox.max = rhs.boundingBox.max;

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
	// Mesh::collection.remove(this);
	Mesh::collection.erase(remove(begin(Mesh::collection), end(Mesh::collection), this), end(Mesh::collection));
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

	boundingRadius = rhs.boundingRadius;
	boundingBox.min = rhs.boundingBox.min;
	boundingBox.max = rhs.boundingBox.max;

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

	SRC_MESH_OPEN(*source);

	const auto &vertexData = source->vertexData;
	const auto &indexData = source->indexData;

	layouts = source->layouts;
	indices = source->indices;
	arrays = source->arrays;

	for (const auto & layout : layouts)
	{
		// clog << layout.index << ", " << layout.size << ", " << rn::getEnumName(layout.type) << ", " << layout.stride << ", " << dec << reinterpret_cast<size_t>(layout.pointer) << " vbo=" << vbo << " ibo=" << ibo << endl;

		if (layout.index == rn::LayoutLocation::pos)
		{
			buildBounds(vertexData, layout);
			break;
		}
	}

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
		clog << "  [Mesh \"" << meshName << "\" {\"" << source->name() << "\"}:" << (ngn::time() - timer) << "s]" << endl;
		clog.unsetf(ios::floatfield);
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

		// clog << meshName << " (" << vao << ") layouts:" << endl;
		for (const auto & layout : layouts)
		{
			// clog << layout.index << ", " << layout.size << ", " << rn::getEnumName(layout.type) << ", " << layout.stride << ", " << dec << reinterpret_cast<size_t>(layout.pointer) << " vbo=" << vbo << " ibo=" << ibo << endl;
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

void Mesh::buildBounds(const rn::BufferData &vertexData, const rn::VertexLayout &layout)
{
	boundingRadius = -1.f;
	boundingBox.min = glm::vec3{numeric_limits<float>::max()};
	boundingBox.max = glm::vec3{-numeric_limits<float>::max()};

	if ( ! vertexData.data) {
		return;
	}

	if (layout.type != GL_FLOAT)
	{
		clog << "rn::Mesh{" + meshName + "}::buildBounds() - unknown layout type" << endl;
		return;
	}

	if (layout.size != 3)
	{
		clog << "rn::Mesh{" + meshName + "}::buildBounds() - unknown layout size" << endl;
		return;
	}

	if (static_cast<size_t>(layout.stride) < sizeof(GLfloat) * 3)
	{
		clog << "rn::Mesh{" + meshName + "}::buildBounds() - incorrect stride" << endl;
		return;
	}

	const uint8_t *data = reinterpret_cast<uint8_t *>(vertexData.data) + reinterpret_cast<size_t>(layout.pointer);
	const uint8_t *end = reinterpret_cast<uint8_t *>(vertexData.data) + vertexData.size;

	while ((data + sizeof(GLfloat) * 3) < end)
	{
		const GLfloat *vert = reinterpret_cast<const GLfloat *>(data);

		boundingBox.min.x = min(boundingBox.min.x, vert[0]);
		boundingBox.max.x = max(boundingBox.max.x, vert[0]);
		boundingBox.min.y = min(boundingBox.min.y, vert[1]);
		boundingBox.max.y = max(boundingBox.max.y, vert[1]);
		boundingBox.min.z = min(boundingBox.min.z, vert[2]);
		boundingBox.max.z = max(boundingBox.max.z, vert[2]);

		boundingRadius = max(boundingRadius, vert[0] * vert[0] + vert[1] * vert[1] + vert[2] * vert[2]);

		data += layout.stride;
	}

	boundingRadius = sqrt(boundingRadius);
}

void Mesh::render()
{
	if (vao)
	{
		RN_CHECK(glBindVertexArray(vao));

		// clog << "vao=" << vao << " {" << meshName << "}" << endl;

		for (const auto &index : indices)
		{
			// clog << "  index.mode=[" << rn::getEnumName(index.mode) << "] index.count=" << index.count << " index.offset=" << index.offset << endl;
			RN_CHECK(glDrawElements(index.mode, index.count, index.type, reinterpret_cast<GLvoid *>(index.offset)));
			rn::stats.triangles += index.count;
		}

		for (const auto &array : arrays)
		{
			// clog << "  array.mode=[" << rn::getEnumName(array.mode) << "] array.offset=" << array.offset << " array.count=" << array.count << endl;
			RN_CHECK_PARAM(glDrawArrays(array.mode, array.offset, array.count), array.mode << " " << rn::getEnumName(array.mode));
			rn::stats.triangles += array.count;
		}

		RN_CHECK(glBindVertexArray(0));
	}
}

namespace
{
	const util::InitQAttacher attach(rn::initQ(), []
	{
		auto &&source = src::mem::mesh({
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
		});

		assert(source.get() != nullptr);

		source->arrays.emplace_back(GL_TRIANGLE_STRIP, 0, 4);
		source->layouts.emplace_back(rn::LayoutLocation::pos, 2, GL_FLOAT, sizeof(GLfloat) * 4, 0);
		source->layouts.emplace_back(rn::LayoutLocation::tex, 2, GL_FLOAT, sizeof(GLfloat) * 4, sizeof(GLfloat) * 2);

		dynamic_cast<src::mem::Mesh *>(source.get())->setName("Mesh quad");
		// reinterpret_cast<src::mem::Mesh *>(source.get())->setName("Mesh quad");

		Mesh::quad.load(move(source));
	});
}

} // rn