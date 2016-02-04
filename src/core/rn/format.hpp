#pragma once

#include "../rn.hpp"

namespace rn
{

struct InternalFormat
{
	GLint layout;
	GLenum components;
};

namespace format
{

extern const rn::InternalFormat R8;
extern const rn::InternalFormat R8SN;
extern const rn::InternalFormat R16;
extern const rn::InternalFormat R16SN;
extern const rn::InternalFormat RG8;
extern const rn::InternalFormat RG8SN;
extern const rn::InternalFormat RG16;
extern const rn::InternalFormat RG16SN;
extern const rn::InternalFormat R3G3B2;
extern const rn::InternalFormat RGB4;
extern const rn::InternalFormat RGB5;
extern const rn::InternalFormat RGB8;
extern const rn::InternalFormat RGB8SN;
extern const rn::InternalFormat RGB10;
extern const rn::InternalFormat RGB12;
extern const rn::InternalFormat RGB16SN;
extern const rn::InternalFormat RGBA2;
extern const rn::InternalFormat RGBA4;
extern const rn::InternalFormat RGB5A1;
extern const rn::InternalFormat RGBA8;
extern const rn::InternalFormat RGBA8SN;
extern const rn::InternalFormat RGB10A2;
extern const rn::InternalFormat RGB10A2UI;
extern const rn::InternalFormat RGBA12;
extern const rn::InternalFormat RGBA16;
extern const rn::InternalFormat SRGB8;
extern const rn::InternalFormat SRGB8A8;
extern const rn::InternalFormat R16F;
extern const rn::InternalFormat RG16F;
extern const rn::InternalFormat RGB16F;
extern const rn::InternalFormat RGBA16F;
extern const rn::InternalFormat R32F;
extern const rn::InternalFormat RG32F;
extern const rn::InternalFormat RGB32F;
extern const rn::InternalFormat RGBA32F;
extern const rn::InternalFormat R11FG11FB10F;
extern const rn::InternalFormat RGB9E5;
extern const rn::InternalFormat R8I;
extern const rn::InternalFormat R8UI;
extern const rn::InternalFormat R16I;
extern const rn::InternalFormat R16UI;
extern const rn::InternalFormat R32I;
extern const rn::InternalFormat R32UI;
extern const rn::InternalFormat RG8I;
extern const rn::InternalFormat RG8UI;
extern const rn::InternalFormat RG16I;
extern const rn::InternalFormat RG16UI;
extern const rn::InternalFormat RG32I;
extern const rn::InternalFormat RG32UI;
extern const rn::InternalFormat RGB8I;
extern const rn::InternalFormat RGB8UI;
extern const rn::InternalFormat RGB16I;
extern const rn::InternalFormat RGB16UI;
extern const rn::InternalFormat RGB32I;
extern const rn::InternalFormat RGB32UI;
extern const rn::InternalFormat RGBA8I;
extern const rn::InternalFormat RGBA8UI;
extern const rn::InternalFormat RGBA16I;
extern const rn::InternalFormat RGBA16UI;
extern const rn::InternalFormat RGBA32I;
extern const rn::InternalFormat RGBA32UI;

extern const rn::InternalFormat D16;
extern const rn::InternalFormat D24;
extern const rn::InternalFormat D32F;
extern const rn::InternalFormat D24S8;
extern const rn::InternalFormat D32FS8;

} // internal

} // rn