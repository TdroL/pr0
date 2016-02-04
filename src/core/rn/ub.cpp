#include "ub.hpp"

namespace rn
{

using namespace std;

UB::UB()
{}

UB::UB(std::string &&ubName)
	: UB{}
{
	this->ubName = move(ubName);
}

UB::~UB()
{
	reset();
}

void * UB::mapData(size_t size)
{
	size_t origSize = data.size();

	if (size)
	{
		data.resize(data.size() + size / sizeof(DataType));
	}

	return &(*(data.begin() + origSize));
}

size_t UB::size()
{
	return data.size();
}

void UB::reserve(size_t size)
{
	data.reserve(size);
}

void UB::clear()
{
	data.clear();
}

void UB::reset()
{
	if (id)
	{
		RN_CHECK(glDeleteBuffers(1, &id));
		id = 0;
	}

	deferBinding = false;
}

void UB::reload()
{
	reset();

	RN_CHECK(glCreateBuffers(1, &id));

	upload();
}

void UB::upload()
{
	if (id)
	{
		RN_CHECK(glInvalidateBufferData(id));

		if (data.size())
		{
			RN_CHECK(glNamedBufferData(id, data.size() * sizeof(DataType), data.data(), GL_DYNAMIC_COPY));

			if (deferBinding)
			{
				bind(bindingIndex);
			}
		}
	}
}

void UB::bind(GLuint bindingIndex)
{
	this->bindingIndex = bindingIndex;

	if (id)
	{
		if (data.size())
		{
			RN_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, id));
			deferBinding = false;
		}
		else
		{
			deferBinding = true;
		}
	}
}

} // rn