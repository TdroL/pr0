#include <pch.hpp>

#include "tex2darray.hpp"

#include "../ngn.hpp"
#include "../rn.hpp"
#include "../rn/ext.hpp"

#include <utility>
#include <iostream>
#include <algorithm>

namespace rn
{

using namespace std;

vector<Tex2DArray *> Tex2DArray::collection{};

void Tex2DArray::reloadAll()
{
	for (Tex2DArray *tex : Tex2DArray::collection)
	{
		try
		{
			tex->reload();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

Tex2DArray::Tex2DArray()
{
	Tex2DArray::collection.push_back(this);
}

Tex2DArray::Tex2DArray(string &&texName)
	: Tex2DArray{}
{
	this->texName = move(texName);
}

Tex2DArray::Tex2DArray(GLint internalFormat/*, unique_ptr<Source> &&source*/)
	: Tex2DArray{}
{
	this->internalFormat = internalFormat;

	// this->source = move(source);
}

Tex2DArray::Tex2DArray(string &&texName, GLint internalFormat/*, unique_ptr<Source> &&source*/)
	: Tex2DArray{}
{
	this->internalFormat = internalFormat;

	// this->source = move(source);
	this->texName = move(texName);
}

Tex2DArray::Tex2DArray(Tex2DArray &&rhs)
	: Tex2DArray{}
{
	id = move(rhs.id);
	width = move(rhs.width);
	height = move(rhs.height);
	size = move(rhs.size);
	levels = move(rhs.levels);
	internalFormat = move(rhs.internalFormat);
	format = move(rhs.format);
	type = move(rhs.type);
	minFilter = move(rhs.minFilter);
	magFilter = move(rhs.magFilter);
	wrapS = move(rhs.wrapS);
	wrapT = move(rhs.wrapT);
	compareFunc = move(rhs.compareFunc);
	// source = move(rhs.source);
	texName = move(rhs.texName);

	rhs.id = 0;
	// rhs.source.reset();
	rhs.reset();
}

Tex2DArray::~Tex2DArray()
{
	reset();

	// Tex2DArray::collection.remove(this);
	Tex2DArray::collection.erase(remove(begin(Tex2DArray::collection), end(Tex2DArray::collection), this), end(Tex2DArray::collection));
}

Tex2DArray & Tex2DArray::operator=(Tex2DArray &&rhs)
{
	reset();

	id = move(rhs.id);
	width = move(rhs.width);
	height = move(rhs.height);
	size = move(rhs.size);
	levels = move(rhs.levels);
	internalFormat = move(rhs.internalFormat);
	format = move(rhs.format);
	type = move(rhs.type);
	minFilter = move(rhs.minFilter);
	magFilter = move(rhs.magFilter);
	wrapS = move(rhs.wrapS);
	wrapT = move(rhs.wrapT);
	compareFunc = move(rhs.compareFunc);
	// source = move(rhs.source);
	texName = move(rhs.texName);

	rhs.id = 0;
	// rhs.source.reset();
	rhs.reset();

	return *this;
}

void Tex2DArray::reload()
{
	double timer = ngn::time();

	reset();

	RN_CHECK(glGenTextures(1, &id));

	RN_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, id));

	RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter), rn::getEnumName(minFilter));
	RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, magFilter), rn::getEnumName(magFilter));
	RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapS), rn::getEnumName(wrapS));
	RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapT), rn::getEnumName(wrapT));

	RN_CHECK(glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor)));

	if (levels > 0)
	{
		RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, levels), levels);
	}

	switch (internalFormat)
	{
		case GL_DEPTH_COMPONENT:
			format = GL_DEPTH_COMPONENT;
			type = GL_UNSIGNED_INT;
		break;
		case GL_DEPTH_COMPONENT16:
			format = GL_DEPTH_COMPONENT;
			type = GL_UNSIGNED_INT;
		break;
		case GL_DEPTH_COMPONENT24:
			format = GL_DEPTH_COMPONENT;
			type = GL_UNSIGNED_INT;
		break;
		case GL_DEPTH_COMPONENT32:
			format = GL_DEPTH_COMPONENT;
			type = GL_UNSIGNED_INT;
		break;
		case GL_DEPTH_COMPONENT32F:
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
		break;
		case GL_DEPTH24_STENCIL8:
			format = GL_DEPTH_STENCIL;
			type = GL_UNSIGNED_INT_24_8;
		break;
		case GL_DEPTH32F_STENCIL8:
			format = GL_DEPTH_STENCIL;
			type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		break;
		default:
			format = GL_RED;
			type = GL_UNSIGNED_BYTE;
	}

	if (isDepth() && compareFunc != COMPARE_NONE) {
		RN_CHECK(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
		RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, compareFunc), rn::getEnumName(compareFunc));
	}

	GLsizei mipWidth = width;
	GLsizei mipHeight = height;

	for (GLint i = 0; i <= levels; i++)
	{
		RN_CHECK_PARAM(glTexImage3D(GL_TEXTURE_2D_ARRAY, i, internalFormat, mipWidth, mipHeight, size, 0, format, type, nullptr), i << ", " << rn::getEnumName(internalFormat) << ", " << mipWidth << ", " << mipHeight << ", " << rn::getEnumName(format) << ", " << rn::getEnumName(type));

		mipWidth = max(1, mipWidth / 2);
		mipHeight = max(1, mipHeight / 2);
	}

	RN_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [Tex2DArray \"" << texName << "\":" << (ngn::time() - timer) << "s]" << endl;
		clog.unsetf(ios::floatfield);
	}
}

void Tex2DArray::reset()
{
	if (id)
	{
		RN_CHECK(glDeleteTextures(1, &id));
		id = 0;
	}
}

bool Tex2DArray::isDepth()
{
	switch (internalFormat)
	{
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32:
		case GL_DEPTH_COMPONENT32F:
		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH32F_STENCIL8:
			return true;
		default:
			return false;
	}
}
bool Tex2DArray::isDepthStencil()
{
	switch (internalFormat)
	{
		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH32F_STENCIL8:
			return true;
		default:
			return false;
	}
}

GLsizei Tex2DArray::bind(GLsizei unit)
{
	RN_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
	RN_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, id));

	return unit;
}

GLenum Tex2DArray::getAttachmentType()
{
	switch (internalFormat)
	{
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32F:
			return GL_DEPTH_ATTACHMENT;

		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH32F_STENCIL8:
			return GL_DEPTH_STENCIL_ATTACHMENT;

		default:
			return GL_COLOR_ATTACHMENT0;
	}
}

} // rn

