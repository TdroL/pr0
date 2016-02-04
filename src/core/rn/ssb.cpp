#include "ssb.hpp"

namespace rn
{

using namespace std;

SSB::SSB()
{}

SSB::SSB(std::string &&ssbName)
	: SSB{}
{
	this->ssbName = move(ssbName);
}

SSB::~SSB()
{
	reset();
}

void * SSB::mapData(size_t size)
{
	size_t origSize = data.size();

	if (size)
	{
		data.resize(data.size() + size / sizeof(DataType));
	}

	return &(*(data.begin() + origSize));
}

size_t SSB::size()
{
	return data.size();
}

void SSB::reserve(size_t size)
{
	data.reserve(size);
}

void SSB::clear()
{
	data.clear();
}

void SSB::reset()
{
	if (id)
	{
		RN_CHECK(glDeleteBuffers(1, &id));
		id = 0;
	}

	deferBinding = false;
}

void SSB::reload()
{
	reset();

	RN_CHECK(glCreateBuffers(1, &id));

	upload();
}

void SSB::upload()
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

void SSB::bind(GLuint bindingIndex)
{
	this->bindingIndex = bindingIndex;

	if (id)
	{
		if (data.size())
		{
			RN_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, id));
			deferBinding = false;
		}
		else
		{
			deferBinding = true;
		}
	}
}

} // rn