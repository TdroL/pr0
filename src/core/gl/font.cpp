#include "font.hpp"
#include "../util.hpp"
#include "../gl.hpp"
#include "program.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "../src/file.hpp"
#include "../sys/window.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <iostream>

namespace gl
{

using namespace std;

FT_Library ft;

list<Font *> Font::collection{};
gl::Program prog{};

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

void Font::init()
{
	if (FT_Init_FreeType(&ft))
	{
		throw string{"gl::Font::init() - could not initialize FreeType library"};
	}

	if ( ! prog.id)
	{
		prog.load(src::file::stream("gl/font.frag"), src::file::stream("gl/font.vert"));
	}
}

void Font::deinit()
{
	prog.reset();
}

Font::Font()
{
	Font::collection.push_back(this);
}

Font::~Font()
{
	Font::collection.remove(this);
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
	reset();

	if ( ! source)
	{
		throw string{"gl::Font::reload() - missing font source"};
	}

	SRC_STREAM_USE(*source);

	FT_Open_Args args;
	args.flags = FT_OPEN_MEMORY;
	args.memory_base = reinterpret_cast<const FT_Byte *>(source->contents.data());
	args.memory_size = source->contents.size() * sizeof(decltype(source->contents)::value_type);

	FT_Error err;

	if ((err = FT_Open_Face(ft, &args, 0, &face)) != 0)
	{
		throw string{"gl::Font::init() - could not load font (error " + to_string(err) + ")"};
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);

	sx = sx ? sx : 2.0f / sys::window::width;
	sy = sy ? sy : 2.0f / sys::window::height;

	GL_CHECK(glGenVertexArrays(1, &vao));
	GL_CHECK(glGenBuffers(1, &vbo));

	GL_CHECK(glGenTextures(1, &tex));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex));
	GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	FT_GlyphSlot g = face->glyph;
	int w = 0;
	int h = 0;

	const int MAXWIDTH = 1024;

	int roww = 0;
	int rowh = 0;

	for (int i = 32; i < 128; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			throw string{"gl::Font::reload() - loading character " + to_string(static_cast<char>(i)) + " failed"};
			continue;
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

	atlas_width = w;
	atlas_height = h;

	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr));

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

		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer));

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
		glDeleteTextures(1, &tex);
		tex = 0;
	}

	GL_CHECK(glDeleteBuffers(1, &vbo));
	GL_CHECK(glDeleteVertexArrays(1, &vao));

	vao = vbo = 0;
}

void Font::render(const string &text)
{
	float x = position.x;
	float y = position.y - lineHeight * sy + max(static_cast<float>(lineHeight - fontSize), 0.f) * 0.5f * sy;

	unique_ptr<glm::vec4[]> coords{new glm::vec4[text.size() * 6]};

	prog.use();

	GL_CHECK(glEnable(GL_BLEND));
	GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex));

	prog.uniform("tex", 0);
	prog.uniform("color", color);

	GL_CHECK(glBindVertexArray(vao));
	GL_CHECK(glEnableVertexAttribArray(0));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));

	GLsizei n = 0;

	auto &aw = atlas_width;
	auto &ah = atlas_height;

	for (int i = 0, size = text.size(); i < size; i++)
	{
		const char c = text[i];

		if (c == '\n')
		{
			x = position.x;
			y -= lineHeight * sy;
			continue;
		}

		const FontChar &fc = chars[c];

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

	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * text.size() * 6, coords.get(), GL_DYNAMIC_DRAW));
	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, n));

	gl::stats.triangles += n;

	glBindVertexArray(0);
	glUseProgram(0);
}


}