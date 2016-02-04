#include <pch.hpp>

#include "obj.hpp"
#include "../rn.hpp"
#include "../rn/types.hpp"
#include "../ngn/fs.hpp"
#include "../util/str.hpp"

#include <array>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

namespace src
{

using namespace std;

namespace fs = ngn::fs;
namespace str = util::str;

namespace obj
{

float normZero(float value)
{
	return value != 0.f ? value : 0.f;
}

Mesh::Mesh(string &&fileName)
	: fileName{move(fileName)}
{
	#if defined(GLM_LEFT_HANDED)
		flipZs = true;
	#else
		flipZs = false;
	#endif
}

void Mesh::open()
{
	vector<glm::vec3> inVertices;
	vector<glm::vec2> inUvs;
	vector<glm::vec3> inNormals;
	map<string, size_t> inIndices;
	vector<array<string, 3>> inFaces;

	fstream ifs{fs::find(fileName), ios::in};

	if ( ! ifs.is_open())
	{
		throw string{"src::obj::Mesh::open() - could not open file \"" + fileName + "\""};
	}

	while ( ! ifs.eof())
	{
		string line;
		getline(ifs, line);
		str::trim(line);

		if (line.empty())
		{
			continue;
		}

		if (line.compare(0, 2, "v ") == 0)
		{
			glm::vec3 vert;
			stringstream{line.substr(2)} >> vert.x >> vert.y >> vert.z;

			if (flipZs)
			{
				vert.z = -vert.z;
			}

			vert.x = normZero(vert.x);
			vert.y = normZero(vert.y);
			vert.z = normZero(vert.z);

			inVertices.push_back(vert);
		}
		else if (line.compare(0, 3, "vt ") == 0)
		{
			glm::vec2 vert;
			stringstream{line.substr(3)} >> vert.x >> vert.y;

			vert.x = normZero(vert.x);
			vert.y = normZero(vert.y);

			inUvs.push_back(vert);
		}
		else if (line.compare(0, 3, "vn ") == 0)
		{
			glm::vec3 vert;
			stringstream{line.substr(3)} >> vert.x >> vert.y >> vert.z;

			if (flipZs)
			{
				vert.z = -vert.z;
			}

			vert.x = normZero(vert.x);
			vert.y = normZero(vert.y);
			vert.z = normZero(vert.z);

			inNormals.push_back(vert);
		}
		else if (line.compare(0, 2, "f ") == 0)
		{
			string part;
			stringstream parser{line.substr(2)};

			decltype(inFaces)::value_type face;

			for (int i = 0; i < 3; ++i)
			{
				parser >> part;

				inIndices[part] = 0;
				face[i] = part;
			}

			inFaces.push_back(face);

			while ( ! parser.eof())
			{
				parser >> part;

				inIndices[part] = 0;

				face[1] = face[2];
				face[2] = part;

				inFaces.push_back(face);
			}
		}
	}

	enum Branch
	{
		vert,
		vert_uv,
		vert_norm,
		vert_uv_norm,
	};

	Branch branch;

	const auto it = begin(inIndices);

	const auto lSlash = it->first.find('/');
	const auto rSlash = it->first.rfind('/');

	if (lSlash == string::npos) // f v1 v2 v3
	{
		branch = Branch::vert;
	}
	else if (lSlash == rSlash) // f v1/vt1 v2/vt2 v3/vt3
	{
		branch = Branch::vert_uv;
	}
	else if (lSlash + 1 == rSlash) // f v1//vn1 v2//vn2 v3//vn3
	{
		branch = Branch::vert_norm;
	}
	else // f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
	{
		branch = Branch::vert_uv_norm;
	}

	size_t estimatedSize = static_cast<size_t>(inFaces.size() * 3 * ((inVertices.size() > 0) * 3 + (inUvs.size() > 0) * 2 + (inNormals.size() > 0) * 3) * 1.5);

	decltype(vertexCache) vertexBuffer{};
	vertexBuffer.reserve(estimatedSize);

	size_t i = 0;
	for (auto &item : inIndices)
	{
		stringstream parser{item.first};
		item.second = i++;

		int vertex = 0;
		int uv = 0;
		int normal = 0;
		char dummy;

		switch (branch)
		{
			case Branch::vert:
			{
				parser >> vertex;
				if ( ! vertex)
				{
					throw string{"src::obj::Mesh::open() - incorrect face: \"" + item.first + "\". Expected: \"[vertex]\""};
				}

				vertex = vertex > 0 ? vertex - 1 : inVertices.size() + vertex;

				if (static_cast<size_t>(vertex) >= inVertices.size())
				{
					throw string{"src::obj::Mesh::open() - illegal vertex index: \"" + to_string(vertex) + "\". Expected number of range from 1 to " + to_string(inVertices.size())};
				}

				vertexBuffer.push_back(inVertices[vertex].x);
				vertexBuffer.push_back(inVertices[vertex].y);
				vertexBuffer.push_back(inVertices[vertex].z);

				break;
			}
			case Branch::vert_uv:
			{
				parser >> vertex >> dummy >> uv;

				if ( ! vertex || ! uv)
				{
					throw string{"src::obj::Mesh::open() - incorrect face: \"" + item.first + "\". Expected: \"[vertex]/[uv]\""};
				}

				vertex = vertex > 0 ? vertex - 1 : inVertices.size() + vertex;
				uv = uv > 0 ? uv - 1 : inUvs.size() + uv;

				if (static_cast<size_t>(vertex) >= inVertices.size())
				{
					throw string{"src::obj::Mesh::open() - illegal vertex index: \"" + to_string(vertex) + "\". Expected number of range from 1 to " + to_string(inVertices.size())};
				}

				if (static_cast<size_t>(uv) >= inUvs.size())
				{
					throw string{"src::obj::Mesh::open() - illegal uv index: \"" + to_string(uv) + "\". Expected number of range from 1 to " + to_string(inUvs.size())};
				}

				vertexBuffer.push_back(inVertices[vertex].x);
				vertexBuffer.push_back(inVertices[vertex].y);
				vertexBuffer.push_back(inVertices[vertex].z);
				vertexBuffer.push_back(inUvs[uv].x);
				vertexBuffer.push_back(inUvs[uv].y);

				break;
			}
			case Branch::vert_norm:
			{

				parser >> vertex >> dummy >> dummy >> normal;

				if ( ! vertex || ! normal)
				{
					throw string{"src::obj::Mesh::open() - incorrect face: \"" + item.first + "\". Expected: \"[vertex]//[normal]\""};
				}

				vertex = vertex > 0 ? vertex - 1 : inVertices.size() + vertex;
				normal = normal > 0 ? normal - 1 : inNormals.size() + normal;

				if (static_cast<size_t>(vertex) >= inVertices.size())
				{
					throw string{"src::obj::Mesh::open() - illegal vertex index: \"" + to_string(vertex) + "\". Expected number of range from 1 to " + to_string(inVertices.size())};
				}

				if (static_cast<size_t>(normal) >= inNormals.size())
				{
					throw string{"src::obj::Mesh::open() - illegal normal index: \"" + to_string(normal) + "\". Expected number of range from 1 to " + to_string(inNormals.size())};
				}

				vertexBuffer.push_back(inVertices[vertex].x);
				vertexBuffer.push_back(inVertices[vertex].y);
				vertexBuffer.push_back(inVertices[vertex].z);
				vertexBuffer.push_back(inNormals[normal].x);
				vertexBuffer.push_back(inNormals[normal].y);
				vertexBuffer.push_back(inNormals[normal].z);

				break;
			}
			case Branch::vert_uv_norm:
			{
				parser >> vertex >> dummy >> uv >> dummy >> normal;

				if ( ! vertex || ! uv || ! normal)
				{
					throw string{"src::obj::Mesh::open() - incorrect face: \"" + item.first + "\". Expected: \"[vertex]/[uv]/[normal]\""};
				}

				vertex = vertex > 0 ? vertex - 1 : inVertices.size() + vertex;
				uv = uv > 0 ? uv - 1 : inUvs.size() + uv;
				normal = normal > 0 ? normal - 1 : inNormals.size() + normal;

				if (static_cast<size_t>(vertex) >= inVertices.size())
				{
					throw string{"src::obj::Mesh::open() - illegal vertex index: \"" + to_string(vertex) + "\". Expected number of range from 1 to " + to_string(inVertices.size())};
				}

				if (static_cast<size_t>(uv) >= inUvs.size())
				{
					throw string{"src::obj::Mesh::open() - illegal uv index: \"" + to_string(uv) + "\". Expected number of range from 1 to " + to_string(inUvs.size())};
				}

				if (static_cast<size_t>(normal) >= inNormals.size())
				{
					throw string{"src::obj::Mesh::open() - illegal normal index: \"" + to_string(normal) + "\". Expected number of range from 1 to "+ to_string(inNormals.size())};
				}

				vertexBuffer.push_back(inVertices[vertex].x);
				vertexBuffer.push_back(inVertices[vertex].y);
				vertexBuffer.push_back(inVertices[vertex].z);
				vertexBuffer.push_back(inUvs[uv].x);
				vertexBuffer.push_back(inUvs[uv].y);
				vertexBuffer.push_back(inNormals[normal].x);
				vertexBuffer.push_back(inNormals[normal].y);
				vertexBuffer.push_back(inNormals[normal].z);
				break;
			}
		}
	}

	vertexCache.swap(vertexBuffer);

	decltype(indexCache) indexBuffer{};
	indexBuffer.reserve(inFaces.size() * 3);

	for (auto &face : inFaces)
	{
		indexBuffer.push_back(inIndices[face[0]]);
		indexBuffer.push_back(inIndices[face[1]]);
		indexBuffer.push_back(inIndices[face[2]]);
	}

	indexCache.swap(indexBuffer);

	vertexData.size = vertexCache.size() * sizeof(vertexCache[0]);
	vertexData.data = vertexCache.data();
	vertexData.usage = GL_STATIC_DRAW;

	indexData.size = indexCache.size() * sizeof(indexCache[0]);
	indexData.data = indexCache.data();
	indexData.usage = GL_STATIC_DRAW;

	indices.clear();
	indices.shrink_to_fit();
	indices.reserve(1);
	indices.emplace_back(
		static_cast<GLenum>(GL_TRIANGLES),
		static_cast<GLsizeiptr>(indexCache.size()),
		static_cast<GLenum>(GL_UNSIGNED_INT),
		static_cast<GLsizeiptr>(0)
	);

	layouts.clear();
	layouts.shrink_to_fit();
	layouts.reserve(3);

	switch (branch)
	{
		case Branch::vert:
		{
			GLsizei stride = 3 * sizeof(GLfloat);

			layouts.emplace_back(rn::LayoutLocation::pos, 3, GL_FLOAT, stride, 0 * sizeof(GLfloat));

			break;
		}
		case Branch::vert_uv:
		{
			GLsizei stride = 5 * sizeof(GLfloat);

			layouts.emplace_back(rn::LayoutLocation::pos, 3, GL_FLOAT, stride, 0 * sizeof(GLfloat));
			layouts.emplace_back(rn::LayoutLocation::tex, 2, GL_FLOAT, stride, 3 * sizeof(GLfloat));

			break;
		}
		case Branch::vert_norm:
		{
			GLsizei stride = 6 * sizeof(GLfloat);

			layouts.emplace_back(rn::LayoutLocation::pos, 3, GL_FLOAT, stride, 0 * sizeof(GLfloat));
			layouts.emplace_back(rn::LayoutLocation::norm, 3, GL_FLOAT, stride, 3 * sizeof(GLfloat));

			break;
		}
		case Branch::vert_uv_norm:
		{
			GLsizei stride = 8 * sizeof(GLfloat);

			layouts.emplace_back(rn::LayoutLocation::pos, 3, GL_FLOAT, stride, 0 * sizeof(GLfloat));
			layouts.emplace_back(rn::LayoutLocation::tex, 2, GL_FLOAT, stride, 3 * sizeof(GLfloat));
			layouts.emplace_back(rn::LayoutLocation::norm, 3, GL_FLOAT, stride, 5 * sizeof(GLfloat));

			break;
		}
	}
}

void Mesh::close()
{
	vertexData.size = 0;
	vertexData.data = nullptr;
	vertexData.usage = GL_NONE;

	indexData.size = 0;
	indexData.data = nullptr;
	indexData.usage = GL_NONE;

	indices.clear();
	indices.shrink_to_fit();

	layouts.clear();
	layouts.shrink_to_fit();

	arrays.clear();
	arrays.shrink_to_fit();

	vertexCache.clear();
	vertexCache.shrink_to_fit();

	indexCache.clear();
	indexCache.shrink_to_fit();
}

string Mesh::name()
{
	return fileName;
}

unique_ptr<src::Mesh> mesh(const string &fileName)
{
	return unique_ptr<src::Mesh>{new Mesh{string{fileName}}};
}

unique_ptr<src::Mesh> mesh(string &&fileName)
{
	return unique_ptr<src::Mesh>{new Mesh{move(fileName)}};
}

} // obj

} // src