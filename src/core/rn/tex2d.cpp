#include <pch.hpp>

#include "tex2d.hpp"

#include "../ngn.hpp"
#include "../rn.hpp"
#include "../rn/ext.hpp"

#include <utility>
#include <iostream>
#include <algorithm>

namespace rn
{

using namespace std;

vector<Tex2D *> Tex2D::collection{};

void Tex2D::reloadAll()
{
	for (Tex2D *tex : Tex2D::collection)
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

Tex2D::Tex2D()
{
	Tex2D::collection.push_back(this);
}

Tex2D::Tex2D(string &&texName)
	: Tex2D{}
{
	this->texName = move(texName);
}

Tex2D::Tex2D(GLint internalFormat, unique_ptr<Source> &&source)
	: Tex2D{}
{
	this->internalFormat = internalFormat;

	this->source = move(source);
}

Tex2D::Tex2D(string &&texName, GLint internalFormat, unique_ptr<Source> &&source)
	: Tex2D{}
{
	this->internalFormat = internalFormat;

	this->source = move(source);
	this->texName = move(texName);
}

Tex2D::Tex2D(Tex2D &&rhs)
	: Tex2D{}
{
	id = move(rhs.id);
	width = move(rhs.width);
	height = move(rhs.height);
	levels = move(rhs.levels);
	internalFormat = move(rhs.internalFormat);
	format = move(rhs.format);
	type = move(rhs.type);
	minFilter = move(rhs.minFilter);
	magFilter = move(rhs.magFilter);
	wrapS = move(rhs.wrapS);
	wrapT = move(rhs.wrapT);
	compareFunc = move(rhs.compareFunc);
	source = move(rhs.source);
	texName = move(rhs.texName);

	rhs.id = 0;
	rhs.source.reset();
	rhs.reset();
}

Tex2D::~Tex2D()
{
	reset();

	// Tex2D::collection.remove(this);
	Tex2D::collection.erase(remove(begin(Tex2D::collection), end(Tex2D::collection), this), end(Tex2D::collection));
}

Tex2D & Tex2D::operator=(Tex2D &&rhs)
{
	reset();

	id = move(rhs.id);
	width = move(rhs.width);
	height = move(rhs.height);
	levels = move(rhs.levels);
	internalFormat = move(rhs.internalFormat);
	format = move(rhs.format);
	type = move(rhs.type);
	minFilter = move(rhs.minFilter);
	magFilter = move(rhs.magFilter);
	wrapS = move(rhs.wrapS);
	wrapT = move(rhs.wrapT);
	compareFunc = move(rhs.compareFunc);
	source = move(rhs.source);
	texName = move(rhs.texName);

	rhs.id = 0;
	rhs.source.reset();
	rhs.reset();

	return *this;
}

void Tex2D::reload()
{
	double timer = ngn::time();

	reset();

	RN_CHECK(glGenTextures(1, &id));

	RN_CHECK(glBindTexture(GL_TEXTURE_2D, id));

	RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter), rn::getEnumName(minFilter));
	RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter), rn::getEnumName(magFilter));
	RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS), rn::getEnumName(wrapS));
	RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT), rn::getEnumName(wrapT));

	if (wrapS == WRAP_BORDER || wrapT == WRAP_BORDER)
	{
		RN_CHECK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor)));
	}

	GLsizei mipWidth = width;
	GLsizei mipHeight = height;

	GLubyte *data = nullptr;
	GLubyte *dataEnd = nullptr;

	if (source)
	{
		SRC_TEX2D_OPEN(*source);

		width = source->width;
		height = source->height;
		levels = source->levels;

		format = source->format;
		type = source->type;

		switch (type)
		{
			case GL_UNSIGNED_BYTE_3_3_2:
			case GL_UNSIGNED_BYTE_2_3_3_REV:
			case GL_UNSIGNED_SHORT_5_6_5:
			case GL_UNSIGNED_SHORT_5_6_5_REV:
			case GL_UNSIGNED_INT_10F_11F_11F_REV:
				format = GL_RGB;
			break;
			case GL_UNSIGNED_SHORT_4_4_4_4:
			case GL_UNSIGNED_SHORT_4_4_4_4_REV:
			case GL_UNSIGNED_SHORT_5_5_5_1:
			case GL_UNSIGNED_SHORT_1_5_5_5_REV:
			case GL_UNSIGNED_INT_8_8_8_8:
			case GL_UNSIGNED_INT_8_8_8_8_REV:
			case GL_UNSIGNED_INT_10_10_10_2:
			case GL_UNSIGNED_INT_2_10_10_10_REV:
			case GL_UNSIGNED_INT_5_9_9_9_REV:
				format = (format == GL_BGRA) ? GL_BGRA : GL_RGBA;
			break;
		}

		data = source->data.get();
		dataEnd = source->data.get() + source->size;
	}

	if ( ! width || ! height)
	{
		RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

		UTIL_DEBUG
		{
			clog << fixed;
			clog << "  [Tex2D \"" << texName << "\" skip {" << (source ? "\"" + source->name() + "\"" : "no source") << ", " << rn::getEnumName(internalFormat) << "}:" << (ngn::time() - timer) << "s]" << endl;
			clog.unsetf(ios::floatfield);
		}

		return;
	}

	if (levels > 0)
	{
		RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels), levels);
	}

	if (data)
	{
		RN_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, source->alignment));

		size_t pixesSize = 1;

		switch (type)
		{
			case GL_BYTE:
			case GL_UNSIGNED_BYTE:
			case GL_UNSIGNED_BYTE_2_3_3_REV:
			case GL_UNSIGNED_BYTE_3_3_2:
				pixesSize = sizeof(GLbyte);
			break;
			case GL_SHORT:
			case GL_UNSIGNED_SHORT:
			case GL_UNSIGNED_SHORT_5_5_5_1:
			case GL_UNSIGNED_SHORT_1_5_5_5_REV:
			case GL_UNSIGNED_SHORT_4_4_4_4:
			case GL_UNSIGNED_SHORT_4_4_4_4_REV:
			case GL_UNSIGNED_SHORT_5_6_5:
			case GL_UNSIGNED_SHORT_5_6_5_REV:
				pixesSize = sizeof(GLshort);
			break;
			case GL_FLOAT:
			case GL_INT:
			case GL_UNSIGNED_INT:
			case GL_UNSIGNED_INT_10_10_10_2:
			case GL_UNSIGNED_INT_10F_11F_11F_REV:
			case GL_UNSIGNED_INT_2_10_10_10_REV:
			case GL_UNSIGNED_INT_8_8_8_8:
				pixesSize = sizeof(GLint);
			break;
		}

		if (isDepth() && compareFunc != COMPARE_NONE) {
			RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
			RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, compareFunc), rn::getEnumName(compareFunc));
		}

		for (GLint i = 0; i <= levels && data < dataEnd; i++)
		{
			RN_CHECK_PARAM(glTexImage2D(GL_TEXTURE_2D, i, internalFormat, mipWidth, mipHeight, 0, format, type, reinterpret_cast<GLvoid *>(data)), i << ", " << rn::getEnumName(internalFormat) << ", " << mipWidth << ", " << mipHeight << ", " << rn::getEnumName(format) << ", " << rn::getEnumName(type) << ", " << reinterpret_cast<void *>(data));

			data += mipWidth * mipHeight * pixesSize;

			mipWidth = max(1, mipWidth / 2);
			mipHeight = max(1, mipHeight / 2);
		}

		RN_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
	}
	else
	{
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
			RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
			RN_CHECK_PARAM(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, compareFunc), rn::getEnumName(compareFunc));
		}

		for (GLint i = 0; i <= levels; i++)
		{
			RN_CHECK_PARAM(glTexImage2D(GL_TEXTURE_2D, i, internalFormat, mipWidth, mipHeight, 0, format, type, nullptr), i << ", " << rn::getEnumName(internalFormat) << ", " << mipWidth << ", " << mipHeight << ", " << rn::getEnumName(format) << ", " << rn::getEnumName(type));

			mipWidth = max(1, mipWidth / 2);
			mipHeight = max(1, mipHeight / 2);
		}
	}

	RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [Tex2D \"" << texName << "\" {" << (source ? "\"" + source->name() + "\"" : "no source") << "}:" << (ngn::time() - timer) << "s]" << endl;
		clog.unsetf(ios::floatfield);
	}
}

void Tex2D::reset()
{
	if (id)
	{
		RN_CHECK(glDeleteTextures(1, &id));
		id = 0;
	}
}

bool Tex2D::isDepth()
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
bool Tex2D::isDepthStencil()
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

GLsizei Tex2D::bind(GLsizei unit)
{
	RN_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
	RN_CHECK(glBindTexture(GL_TEXTURE_2D, id));

	return unit;
}

GLenum Tex2D::getAttachmentType()
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

