#include "tex2d.hpp"
#include "../rn.hpp"
#include "../rn/ext.hpp"
#include "../ngn.hpp"

#include <string>

namespace rn
{

using namespace std;

list<Tex2D *> Tex2D::collection{};

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

Tex2D::Tex2D(GLint internalFormat, unique_ptr<Source> &&source)
	: Tex2D{}
{
	this->internalFormat = internalFormat;

	this->source = move(source);
}

Tex2D::Tex2D(GLint internalFormat, GLint filter, GLint wrap, unique_ptr<Source> &&source)
	: Tex2D{}
{
	this->internalFormat = internalFormat;

	this->filter = filter;
	this->wrap = wrap;

	this->source = move(source);
}

Tex2D::Tex2D(string &&texName, GLint internalFormat, unique_ptr<Source> &&source)
	: Tex2D{}
{
	this->internalFormat = internalFormat;

	this->source = move(source);
	this->texName = move(texName);
}

Tex2D::Tex2D(string &&texName, GLint internalFormat, GLint filter, GLint wrap, unique_ptr<Source> &&source)
	: Tex2D{}
{
	this->internalFormat = internalFormat;

	this->filter = filter;
	this->wrap = wrap;

	this->source = move(source);
	this->texName = move(texName);
}


Tex2D::Tex2D(Tex2D &&rhs)
	: Tex2D{}
{
	this->id = rhs.id;
	this->width = rhs.width;
	this->height = rhs.height;
	this->levels = rhs.levels;
	this->filter = rhs.filter;
	this->wrap = rhs.wrap;
	this->internalFormat = rhs.internalFormat;
	this->format = rhs.format;
	this->type = rhs.type;

	this->source = move(rhs.source);
	this->texName = move(rhs.texName);

	rhs.id = 0;
	rhs.source.reset();
	rhs.reset();
}

Tex2D::~Tex2D()
{
	reset();

	Tex2D::collection.remove(this);
}

Tex2D & Tex2D::operator=(Tex2D &&rhs)
{
	reset();

	this->id = rhs.id;
	this->width = rhs.width;
	this->height = rhs.height;
	this->levels = rhs.levels;
	this->filter = rhs.filter;
	this->wrap = rhs.wrap;
	this->internalFormat = rhs.internalFormat;
	this->format = rhs.format;
	this->type = rhs.type;

	this->source = move(rhs.source);

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

	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));

	if (!source)
	{
		if (width && height) {
			RN_CHECK_PARAM(glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height), rn::getEnumName(internalFormat));
		}

		RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		return;
	}

	SRC_TEX2D_OPEN(*source);

	width = source->width;
	height = source->height;
	levels = source->levels;

	format = source->format;
	type = source->type;

	switch (type) {
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

	GLsizei levelWidth = width;
	GLsizei levelHeight = height;

	GLubyte *data = source->data.get();
	GLubyte *dataEnd = source->data.get() + source->size;

	RN_CHECK_PARAM(glTexStorage2D(GL_TEXTURE_2D, levels, internalFormat, width, height), levels << " " << rn::getEnumName(internalFormat) << " " << width << " " << height);

	if (data)
	{
		RN_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, source->alignment));

		size_t pixesSize = 1;

		switch (type) {
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

		for (GLint i = 0; i < levels && data < dataEnd; i++)
		{
			RN_CHECK_PARAM(glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, levelWidth, levelHeight, format, type, reinterpret_cast<GLvoid *>(data)), i << ", " << levelWidth << ", " << levelHeight << ", " << rn::getEnumName(format) << ", " << rn::getEnumName(type) << ", " << reinterpret_cast<void *>(data));

			data += levelWidth * levelHeight * pixesSize;

			levelWidth = max(1, levelWidth / 2);
			levelHeight = max(1, levelHeight / 2);
		}

		RN_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
	}

	RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [Tex2D:" << texName << " {" << source->name() << "}:" << (ngn::time() - timer) << "s]" << endl;
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

namespace
{
	const util::InitQAttacher attach(rn::initQ(), []
	{
		if ( ! rn::ext::ARB_texture_storage) {
			throw string{"rn::Tex2D initQ - rn::Tex2D requires GL_ARB_texture_storage"};
		}
	});
}

} // rn

