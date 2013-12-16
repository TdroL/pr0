#include "sbm.hpp"
#include "../gl.hpp"
#include "../sys/fs.hpp"

#include <array>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace src
{

using namespace std;

namespace fs = sys::fs;

namespace sbm
{

void ltrim(string &s) {
	s.erase(begin(s), find_if(begin(s), end(s), not1(ptr_fun<int, int>(isspace))));
}

void rtrim(string &s) {
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), end(s));
}

void trim(string &s) {
	ltrim(s);
	rtrim(s);
}

float normZero(float value)
{
	return value != 0.f ? value : 0.f;
}

Mesh::Mesh(string &&fileName)
	: fileName{move(fileName)}
{
}

void Mesh::use()
{
	dataCache = fs::contents<vector<char>>(fileName);
	char *data = dataCache.data();
	const char code[4]{'S', 'B', 'M', ' '};

	const size_t fileSize = dataCache.size();
	const char *dataEnd = data + fileSize;

	if (*reinterpret_cast<const uint32_t *>(code) != *reinterpret_cast<const uint32_t *>(data))
	{
		throw string{"src::sbm::Mesh::use() - invalid SBM file \"" + fileName + "\""};
	}

	data += sizeof(uint32_t);

	if (*reinterpret_cast<const uint32_t *>(data) != 0x0001u)
	{
		throw string{"src::sbm::Mesh::use() - unsupported SBM file \"" + fileName + "\""};
	}

	data += sizeof(uint32_t);

	struct SBMHeader
	{
		uint32_t size;
		uint32_t type;
	};

	SBMHeader *header;

	const uint32_t typeVertices = 0x0001;
	const uint32_t typeIndices =  0x0002;
	const uint32_t typeLayouts =  0x0003;
	const uint32_t typeArrays =   0x0004;

	while (data < dataEnd)
	{
		header = reinterpret_cast<SBMHeader *>(data);

		data += sizeof(header->size);

		if (header->size == 0)
		{
			continue;
		}

		switch (header->type)
		{
			case typeVertices:
			{
				size_t size = header->size - sizeof(header->type);

				vertexData.size = size;
				vertexData.data = reinterpret_cast<GLvoid *>(data + sizeof(header->type));
				vertexData.usage = GL_STATIC_DRAW;

				break;
			}
			case typeIndices:
			{
				size_t size = header->size - sizeof(header->type);

				indexData.size = size;
				indexData.data = reinterpret_cast<GLvoid *>(data + sizeof(header->type));
				indexData.usage = GL_STATIC_DRAW;

				indices.clear();
				indices.shrink_to_fit();
				indices.reserve(1);
				indices.emplace_back(static_cast<GLenum>(GL_TRIANGLES), static_cast<GLsizeiptr>(size / sizeof(GLuint)), static_cast<GLenum>(GL_UNSIGNED_INT), static_cast<GLsizeiptr>(0));

				break;
			}
			case typeLayouts:
			{
				size_t size = header->size - sizeof(header->type);

				const decltype(layouts)::value_type *layout = reinterpret_cast<decltype(layouts)::value_type *>(data + sizeof(header->type));
				const size_t n = size / sizeof(*layout);

				layouts.clear();
				layouts.shrink_to_fit();
				layouts.reserve(n);

				for (size_t i = 0; i < n; ++i)
				{
					layouts.push_back(*layout);

					layout += 1;
				}

				break;
			}
			case typeArrays:
			{
				size_t size = header->size - sizeof(header->type);

				const decltype(arrays)::value_type *array = reinterpret_cast<decltype(arrays)::value_type *>(data + sizeof(header->type));
				const size_t n = size / sizeof(*array);

				arrays.clear();
				arrays.shrink_to_fit();
				arrays.reserve(n);

				for (size_t i = 0; i < n; ++i)
				{
					arrays.push_back(*array);

					array += 1;
				}

				break;
			}
		}

		data += header->size;
	}
}

void Mesh::release()
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

	dataCache.clear();
	dataCache.shrink_to_fit();
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


} // sbm

} // src