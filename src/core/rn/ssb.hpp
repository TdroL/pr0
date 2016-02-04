#pragma once

#include <vector>
#include <string>
#include <memory>

#include "../rn.hpp"

namespace rn
{

class SSB
{
public:
	typedef uint8_t DataType;

	GLuint id = 0;
	GLuint bindingIndex = TypeLimit<GLuint>::max;

	bool deferBinding = false;

	std::vector<DataType> data{};

	std::string ssbName = "Unnamed SSB";

	SSB();
	explicit SSB(std::string &&ssbName);
	~SSB();

	void * mapData(size_t size);

	template<typename U>
	U * mapData()
	{
		return reinterpret_cast<U *>(mapData(sizeof(U)));
	}

	size_t size();

	void reserve(size_t size);

	template<typename U>
	size_t appendData(U a)
	{
		size_t currentOffset = data.size();

		const DataType *itFirst = reinterpret_cast<const DataType *>(&a);
		const DataType *itLast = itFirst + sizeof(a) / sizeof(DataType);

		data.insert(data.end(), itFirst, itLast);

		return currentOffset;
	}

	template<typename U>
	size_t appendData(const std::vector<U> & container)
	{
		size_t currentOffset = data.size();

		const DataType *itFirst = reinterpret_cast<const DataType *>(&(*container.begin()));
		const DataType *itLast = reinterpret_cast<const DataType *>(&(*container.end()));

		data.insert(data.end(), itFirst, itLast);

		return currentOffset;
	}

	template<typename U, typename... V>
	size_t appendData(U a, V... rest)
	{
		size_t currentOffset = appendData(a);
		appendData(rest...);
		return currentOffset;
	}

	void clear();

	void reload();

	void upload();

	void reset();

	void bind(GLuint bindingIndex);
};

} // rn