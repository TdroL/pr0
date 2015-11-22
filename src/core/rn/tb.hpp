#pragma once

#include <vector>
#include <string>
#include <memory>

#include "../rn.hpp"
#include "format.hpp"

namespace rn
{

class TB
{
public:
	// static std::vector<TB *> collection;
	// static std::vector<TB *> activeStack;

	// static void reloadAll();
	// static void reloadSoftAll();

	typedef uint8_t DataType;

	GLuint id = 0;
	GLuint tex = 0;

	GLint internalFormat = format::RGBA32F.layout;

	std::vector<DataType> data{};

	std::string tbName = "Unnamed TB";

	TB();
	explicit TB(std::string &&tbName);
	TB(GLint internalFormat);
	TB(std::string &&tbName, GLint internalFormat);
	~TB();

	template<typename U>
	size_t appendData(U a)
	{
		size_t ptr = data.size();

		DataType *itFirst = reinterpret_cast<DataType *>(&a);
		DataType *itLast = itFirst + sizeof(a) / sizeof(DataType);

		data.insert(data.end(), itFirst, itLast);

		return ptr;
	}

	template<typename U, typename... V>
	size_t appendData(U head, V... rest)
	{
		size_t ptr = appendData(head);
		appendData(rest...);
		return ptr;
	}

	void clear();

	void reload();

	void upload();

	void reset();

	GLsizei bind(GLsizei unit);
};

} // rn