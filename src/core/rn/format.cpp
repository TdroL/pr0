#include <pch.hpp>

#include "format.hpp"

namespace rn
{

namespace format
{

const rn::InternalFormat R8            = { GL_R8,                 GL_RED };
const rn::InternalFormat R8SN          = { GL_R8_SNORM,           GL_RED };
const rn::InternalFormat R16           = { GL_R16,                GL_RED };
const rn::InternalFormat R16SN         = { GL_R16_SNORM,          GL_RED };
const rn::InternalFormat RG8           = { GL_RG8,                GL_RG };
const rn::InternalFormat RG8SN         = { GL_RG8_SNORM,          GL_RG };
const rn::InternalFormat RG16          = { GL_RG16,               GL_RG };
const rn::InternalFormat RG16SN        = { GL_RG16_SNORM,         GL_RG };
const rn::InternalFormat R3G3B2        = { GL_R3_G3_B2,           GL_RGB };
const rn::InternalFormat RGB4          = { GL_RGB4,               GL_RGB };
const rn::InternalFormat RGB5          = { GL_RGB5,               GL_RGB };
const rn::InternalFormat RGB8          = { GL_RGB8,               GL_RGB };
const rn::InternalFormat RGB8SN        = { GL_RGB8_SNORM,         GL_RGB };
const rn::InternalFormat RGB10         = { GL_RGB10,              GL_RGB };
const rn::InternalFormat RGB12         = { GL_RGB12,              GL_RGB };
const rn::InternalFormat RGB16SN       = { GL_RGB16_SNORM,        GL_RGB };
const rn::InternalFormat RGBA2         = { GL_RGBA2,              GL_RGB };
const rn::InternalFormat RGBA4         = { GL_RGBA4,              GL_RGB };
const rn::InternalFormat RGB5A1        = { GL_RGB5_A1,            GL_RGBA };
const rn::InternalFormat RGBA8         = { GL_RGBA8,              GL_RGBA };
const rn::InternalFormat RGBA8SN       = { GL_RGBA8_SNORM,        GL_RGBA };
const rn::InternalFormat RGB10A2       = { GL_RGB10_A2,           GL_RGBA };
const rn::InternalFormat RGB10A2UI     = { GL_RGB10_A2UI,         GL_RGBA };
const rn::InternalFormat RGBA12        = { GL_RGBA12,             GL_RGBA };
const rn::InternalFormat RGBA16        = { GL_RGBA16,             GL_RGBA };
const rn::InternalFormat SRGB8         = { GL_SRGB8,              GL_RGB };
const rn::InternalFormat SRGB8A8       = { GL_SRGB8_ALPHA8,       GL_RGBA };
const rn::InternalFormat R16F          = { GL_R16F,               GL_RED };
const rn::InternalFormat RG16F         = { GL_RG16F,              GL_RG };
const rn::InternalFormat RGB16F        = { GL_RGB16F,             GL_RGB };
const rn::InternalFormat RGBA16F       = { GL_RGBA16F,            GL_RGBA };
const rn::InternalFormat R32F          = { GL_R32F,               GL_RED };
const rn::InternalFormat RG32F         = { GL_RG32F,              GL_RG };
const rn::InternalFormat RGB32F        = { GL_RGB32F,             GL_RGB };
const rn::InternalFormat RGBA32F       = { GL_RGBA32F,            GL_RGBA };
const rn::InternalFormat R11FG11FB10F  = { GL_R11F_G11F_B10F,     GL_RGB };
const rn::InternalFormat RGB9E5        = { GL_RGB9_E5,            GL_RGB };
const rn::InternalFormat R8I           = { GL_R8I,                GL_RED };
const rn::InternalFormat R8UI          = { GL_R8UI,               GL_RED };
const rn::InternalFormat R16I          = { GL_R16I,               GL_RED };
const rn::InternalFormat R16UI         = { GL_R16UI,              GL_RED };
const rn::InternalFormat R32I          = { GL_R32I,               GL_RED };
const rn::InternalFormat R32UI         = { GL_R32UI,              GL_RED };
const rn::InternalFormat RG8I          = { GL_RG8I,               GL_RG };
const rn::InternalFormat RG8UI         = { GL_RG8UI,              GL_RG };
const rn::InternalFormat RG16I         = { GL_RG16I,              GL_RG };
const rn::InternalFormat RG16UI        = { GL_RG16UI,             GL_RG };
const rn::InternalFormat RG32I         = { GL_RG32I,              GL_RG };
const rn::InternalFormat RG32UI        = { GL_RG32UI,             GL_RG };
const rn::InternalFormat RGB8I         = { GL_RGB8I,              GL_RGB };
const rn::InternalFormat RGB8UI        = { GL_RGB8UI,             GL_RGB };
const rn::InternalFormat RGB16I        = { GL_RGB16I,             GL_RGB };
const rn::InternalFormat RGB16UI       = { GL_RGB16UI,            GL_RGB };
const rn::InternalFormat RGB32I        = { GL_RGB32I,             GL_RGB };
const rn::InternalFormat RGB32UI       = { GL_RGB32UI,            GL_RGB };
const rn::InternalFormat RGBA8I        = { GL_RGBA8I,             GL_RGBA };
const rn::InternalFormat RGBA8UI       = { GL_RGBA8UI,            GL_RGBA };
const rn::InternalFormat RGBA16I       = { GL_RGBA16I,            GL_RGBA };
const rn::InternalFormat RGBA16UI      = { GL_RGBA16UI,           GL_RGBA };
const rn::InternalFormat RGBA32I       = { GL_RGBA32I,            GL_RGBA };
const rn::InternalFormat RGBA32UI      = { GL_RGBA32UI,           GL_RGBA };

const rn::InternalFormat D16           = { GL_DEPTH_COMPONENT16,  GL_DEPTH_COMPONENT };
const rn::InternalFormat D24           = { GL_DEPTH_COMPONENT24,  GL_DEPTH_COMPONENT };
const rn::InternalFormat D32F          = { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT };
const rn::InternalFormat D24S8         = { GL_DEPTH24_STENCIL8,   GL_DEPTH_STENCIL };
const rn::InternalFormat D32FS8        = { GL_DEPTH32F_STENCIL8,  GL_DEPTH_STENCIL };

} // format

} // rn