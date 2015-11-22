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
	RN_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, id)); // initialize
	RN_CHECK(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

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