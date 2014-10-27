#include "tex2d.hpp"
#include "../rn.hpp"
#include "../rn/ext.hpp"
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

Tex2D::Tex2D(GLint internalFormat, std::unique_ptr<Source> &&source)
	: Tex2D{}
{
	this->internalFormat = internalFormat;

	this->source = move(source);
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
	reset();

	if (!source)
	{
		return;
	}

	SRC_TEX2D_OPEN(*source);

	RN_CHECK(glGenTextures(1, &id));

	RN_CHECK(glBindTexture(GL_TEXTURE_2D, id));

	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter));
	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter));
	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap));
	RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap));


	width = source->width;
	height = source->height;
	levels = source->levels;

	format = source->format;
	type = source->type;

	RN_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, source->alignment));

	RN_CHECK_PARAM(glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height), rn::getEnumName(internalFormat));

	GLsizei levelWidth = width;
	GLsizei levelHeight = height;

	GLbyte *data = source->data.data();

	if (data)
	{
		for (GLint i = 0; i < source->levels; i++)
		{
			RN_CHECK_PARAM(glTexImage2D(GL_TEXTURE_2D, i, internalFormat, levelWidth, levelHeight, 0, format, type, reinterpret_cast<GLvoid *>(data)), rn::getEnumName(internalFormat) << ", " << rn::getEnumName(format) << ", " << rn::getEnumName(type));

			data += levelWidth * levelHeight;

			levelWidth = max(1, levelWidth / 2);
			levelHeight = max(1, levelHeight / 2);
		}
	}

	RN_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));

	RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
}

void Tex2D::reset()
{
	if (id)
	{
		RN_CHECK(glDeleteTextures(1, &id));
		id = 0;
	}
}

void Tex2D::bind(GLsizei unit)
{
	RN_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
	RN_CHECK(glBindTexture(GL_TEXTURE_2D, id));
}

GLenum Tex2D::getAttachmentType()
{
	switch (internalFormat)
	{
		case GL_DEPTH_COMPONENT:
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24:
		case GL_DEPTH_COMPONENT32F:
		{
			return GL_DEPTH_ATTACHMENT;
		}
		case GL_DEPTH24_STENCIL8:
		case GL_DEPTH32F_STENCIL8:
		{
			return GL_DEPTH_STENCIL_ATTACHMENT;
		}
		default:
		{
			return GL_COLOR_ATTACHMENT0;
		}
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

