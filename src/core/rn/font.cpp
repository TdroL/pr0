#include <pch.hpp>

#include "font.hpp"

#include "mesh.hpp"
#include "program.hpp"
#include "../ngn.hpp"
#include "../ngn/window.hpp"
#include "../rn.hpp"
#include "../rn/ext.hpp"
#include "../src/file.hpp"
#include "../util.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

namespace rn
{

namespace win = ngn::window;

using namespace std;

namespace
{
	FT_Library ft;
	rn::Program prog{"rn::Font::prog"};
}

vector<Font *> Font::collection{};
event::Listener<win::WindowResizeEvent> Font::listenerWindowResize{};

void Font::reloadAll()
{
	for (Font *font : Font::collection)
	{
		try
		{
			font->reload();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

void Font::reloadSoftAll()
{
	for (Font *font : Font::collection)
	{
		try
		{
			font->reloadSoft();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

void Font::init()
{
	if (FT_Init_FreeType(&ft))
	{
		throw string{"rn::Font::init() - could not initialize FreeType library"};
	}

	if ( ! prog.id)
	{
		prog.load("rn/font.frag", "rn/font.vert");
	}

	listenerWindowResize.attach([] (const win::WindowResizeEvent &)
	{
		for (Font *font : Font::collection)
		{
			font->sx = win::internalWidth ? 2.0f / win::internalWidth : 0.f;
			font->sy = win::internalHeight ? 2.0f / win::internalHeight : 0.f;
		}
	});
}

Font::Font()
{
	Font::collection.push_back(this);
}

Font::Font(string &&name)
	: Font{}
{
	fontName = move(name);
}

Font::~Font()
{
	// Font::collection.remove(this);
	Font::collection.erase(remove(begin(Font::collection), end(Font::collection), this), end(Font::collection));
}

void Font::load(std::string &&source)
{
	this->source = src::file::stream(source);

	reload();
}

void Font::load(Source *source)
{
	this->source.reset(source);

	reload();
}

void Font::load(unique_ptr<Source> &&source)
{
	this->source = move(source);

	reload();
}

void Font::reload()
{
	if (rn::status != rn::Status::inited)
	{
		throw string{"rn::Font::reload() - OpenGL not initialized"};
	}

	reset();

	double timer = ngn::time();

	if ( ! source)
	{
		throw string{"rn::Font::reload() - missing font source"};
	}

	SRC_STREAM_OPEN(source);

	FT_Open_Args args;
	args.flags = FT_OPEN_MEMORY;
	args.memory_base = reinterpret_cast<const FT_Byte *>(source->contents.data());
	args.memory_size = source->contents.size() * sizeof(decltype(source->contents)::value_type);

	FT_Error err;

	if ((err = FT_Open_Face(ft, &args, 0, &face)) != 0)
	{
		throw string{"rn::Font::reload() - could not load font (error " + to_string(err) + ")"};
	}

	assert(face);

	FT_Select_Charmap(face, FT_ENCODING_UNICODE);

	FT_Set_Pixel_Sizes(face, 0, fontSize);

	sx = 2.0f / win::internalWidth;
	sy = 2.0f / win::internalHeight;

	// RN_CHECK(glGenVertexArrays(1, &vao));
	// RN_CHECK(glGenBuffers(1, &vbo));

	// RN_CHECK(glCreateVertexArrays(1, &vao));
	RN_CHECK(glCreateBuffers(1, &vbo));
	reloadSoft();

	// clog << "rn::Font::reload() - " << fontName << " vao=" << vao << " vbo=" << vbo << endl;

	// RN_CHECK(glGenTextures(1, &tex));
	// RN_CHECK(glBindTexture(GL_TEXTURE_2D, tex));
	// RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	// RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	// RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	// RN_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	RN_CHECK(glCreateTextures(GL_TEXTURE_2D, 1, &tex));
	RN_CHECK(glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	RN_CHECK(glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	RN_CHECK(glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	RN_CHECK(glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	RN_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

	assert(face->glyph);

	const auto g = face->glyph;
	int w = 0;
	int h = 0;

	const int MAXWIDTH = 1024;

	int roww = 0;
	int rowh = 0;

	for (int i = 32; i < 128; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			throw string{"rn::Font::reload() - loading character " + to_string(static_cast<char>(i)) + " failed"};
			// continue;
		}

		if (roww + g->bitmap.width + 1 >= MAXWIDTH)
		{
			w = max(w, roww);
			h += rowh;
			roww = 0;
			rowh = 0;
		}

		roww += g->bitmap.width + 1;
		rowh = max(rowh, g->bitmap.rows);
	}

	w = max(w, roww);
	h += rowh;

	atlasWidth = w;
	atlasHeight = h;

	// RN_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr));
	RN_CHECK_PARAM(glTextureStorage2D(tex, 1, GL_R8, w, h), rn::getEnumName(GL_R8));

	int ox = 0;
	int oy = 0;

	rowh = 0;

	chars.reserve(256);

	for (int i = 32; i < 128; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			continue;
		}

		if (ox + g->bitmap.width + 1 >= MAXWIDTH)
		{
			oy += rowh;
			rowh = 0;
			ox = 0;
		}

		if (g->bitmap.buffer)
		{
			// RN_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer));
			RN_CHECK(glTextureSubImage2D(tex, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer));
		}
		else
		{
			// clog << i << " [" << static_cast<char>(i) << "] skipping glTexSubImage2D" << endl;
		}

		auto &c = chars[i];

		c.ax = g->advance.x >> 6;
		c.ay = g->advance.y >> 6;
		c.bw = g->bitmap.width;
		c.bh = g->bitmap.rows;
		c.bl = g->bitmap_left;
		c.bt = g->bitmap_top;
		c.tx = static_cast<float>(ox) / static_cast<float>(w);
		c.ty = static_cast<float>(oy) / static_cast<float>(h);

		rowh = max(rowh, g->bitmap.rows);
		ox += g->bitmap.width + 1;
	}

	RN_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
	// RN_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [Font \"" << fontName << "\" {" << source->name() << "}:" << (ngn::time() - timer) << "s]" << endl;
		clog.unsetf(ios::floatfield);
	}
}

void Font::reloadSoft()
{
	// RN_CHECK(glGenVertexArrays(1, &vao));
	RN_CHECK(glCreateVertexArrays(1, &vao));

	GLuint index = 0;
	RN_CHECK(glVertexArrayAttribBinding(vao, index, 0));
	RN_CHECK(glVertexArrayAttribFormat(vao, index, 4, GL_FLOAT, GL_FALSE, 0));
	RN_CHECK(glEnableVertexArrayAttrib(vao, index));

	RN_CHECK(glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(glm::vec4)));
}

void Font::reset()
{
	if (face)
	{
		FT_Done_Face(face);
		face = nullptr;
	}

	if (tex)
	{
		RN_CHECK(glDeleteTextures(1, &tex));
		tex = 0;
	}

	if (vbo)
	{
		RN_CHECK(glDeleteBuffers(1, &vbo));
		vbo = 0;
	}

	if (vao)
	{
		RN_CHECK(glDeleteVertexArrays(1, &vao));
		vao = 0;
	}
}

void Font::render(const string &text)
{
	float x = position.x;
	float y = position.y - lineHeight * sy + max(static_cast<float>(lineHeight - fontSize), 0.f) * 0.5f * sy;

	unique_ptr<glm::vec4[]> coords{new glm::vec4[text.size() * 6]};

	prog.use();

	// RN_CHECK(glActiveTexture(GL_TEXTURE0 + 0));
	// RN_CHECK(glBindTexture(GL_TEXTURE_2D, tex));
	GLuint unit = 0;
	RN_CHECK(glBindTextureUnit(unit, tex));
	prog.uniform<GLint>("tex", unit);
	prog.uniform("color", color);

	// RN_CHECK(glBindVertexArray(vao));
	// RN_CHECK(glEnableVertexAttribArray(0));
	// RN_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	// RN_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));


	// glVertexAttribPointer(semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), BUFFER_OFFSET(0));

	// glEnableVertexAttribArray(VertexArrayName, semantic::attr::POSITION);
	// glVertexArrayAttribBinding(VertexArrayName, semantic::attr::POSITION, 0);
	// glVertexArrayAttribFormat(VertexArrayName, semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, 0);

	GLsizei n = 0;

	auto &aw = atlasWidth;
	auto &ah = atlasHeight;

	for (int i = 0, size = text.size(); i < size; i++)
	{
		const char c = text[i];

		if (c == '\n')
		{
			x = position.x;
			y -= lineHeight * sy;
			continue;
		}

		const auto &fc = chars[c];

		float x2 =  x + fc.bl * sx;
		float y2 = -y - fc.bt * sy;
		float w  = fc.bw * sx;
		float h  = fc.bh * sy;

		x += fc.ax * sx;
		y += fc.ay * sy;

		if ( ! w || ! h)
		{
			continue;
		}

		coords[n++] = glm::vec4{x2,     -y2,     fc.tx,              fc.ty};
		coords[n++] = glm::vec4{x2 + w, -y2,     fc.tx + fc.bw / aw, fc.ty};
		coords[n++] = glm::vec4{x2,     -y2 - h, fc.tx,              fc.ty + fc.bh / ah};
		coords[n++] = glm::vec4{x2 + w, -y2,     fc.tx + fc.bw / aw, fc.ty};
		coords[n++] = glm::vec4{x2,     -y2 - h, fc.tx,              fc.ty + fc.bh / ah};
		coords[n++] = glm::vec4{x2 + w, -y2 - h, fc.tx + fc.bw / aw, fc.ty + fc.bh / ah};
	}

	RN_CHECK(glInvalidateBufferData(vbo));
	RN_CHECK(glNamedBufferData(vbo, sizeof(glm::vec4) * text.size() * 6, coords.get(), GL_DYNAMIC_DRAW));

	RN_CHECK(glBindVertexArray(vao));
	RN_CHECK(glDrawArrays(GL_TRIANGLES, 0, n));
	RN_CHECK(glBindVertexArray(0));

	rn::stats.triangles += n;

	prog.forgo();
}

namespace
{
	const util::InitQAttacher attach(rn::initQ(), []
	{
		/*
		if ( ! rn::ext::ARB_texture_storage)
		{
			throw string{"rn::Font initQ - rn::Font requires GL_ARB_texture_storage"};
		}

		if ( ! rn::ext::ARB_direct_state_access)
		{
			throw string{"rn::Font initQ - rn::Font requires GL_ARB_direct_state_access"};
		}
		*/

		rn::Font::init();
	});
}

}