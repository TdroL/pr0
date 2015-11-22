#include "tb.hpp"

namespace rn
{

using namespace std;

TB::TB()
{}

TB::TB(std::string &&tbName)
	: TB{}
{
	this->tbName = move(tbName);
}

TB::TB(GLint internalFormat)
	: TB{}
{
	this->internalFormat = internalFormat;
}

TB::TB(std::string &&tbName, GLint internalFormat)
	: TB{}
{
	this->tbName = move(tbName);
	this->internalFormat = internalFormat;
}

TB::~TB()
{
	reset();
}

void TB::clear()
{
	data.clear();
}

void TB::reset()
{
	if (tex)
	{
		RN_CHECK(glDeleteTextures(1, &tex));
		tex = 0;
	}

	if (id)
	{
		RN_CHECK(glDeleteBuffers(1, &id));
		id = 0;
	}
}

void TB::reload()
{
	reset();

	// RN_CHECK(glGenBuffers(1, &id));
	// RN_CHECK(glGenTextures(1, &tex));

	RN_CHECK(glCreateBuffers(1, &id));
	RN_CHECK(glCreateTextures(GL_TEXTURE_BUFFER, 1, &tex));

	upload();

	// RN_CHECK(glBindTexture(GL_TEXTURE_BUFFER, tex));
	// RN_CHECK(glTexBuffer(GL_TEXTURE_BUFFER, internalFormat, id));
	// RN_CHECK(glBindTexture(GL_TEXTURE_BUFFER, 0));
	RN_CHECK(glTextureBuffer(tex, internalFormat, id));
}

void TB::upload()
{
	if (id)
	{
		RN_CHECK(glInvalidateBufferData(id));
		RN_CHECK(glNamedBufferData(id, data.size() * sizeof(DataType), data.data(), GL_DYNAMIC_COPY));
	}
}

GLsizei TB::bind(GLsizei unit)
{
	if (tex)
	{
		RN_CHECK(glBindTextureUnit(unit, tex));
	}

	return unit;
}

} // rn