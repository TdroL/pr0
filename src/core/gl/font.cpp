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

float sx = 2.0f / 1600.0f;
float sy = 2.0f /  900.0f;

void Font::reloadAll()
{
	sx = 2.0f / sys::window::width;
	sy = 2.0f / sys::window::height;

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

	sx = 2.0f / sys::window::width;
	sy = 2.0f / sys::window::height;
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

	GL_CHECK(glGenVertexArrays(1, &vao));
	GL_CHECK(glGenBuffers(1, &vbo));
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
	const char *p;
	FT_GlyphSlot g = face->glyph;

	prog.use();

	/* Create a texture that will be used to hold one "glyph" */
	GLuint tex;
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	GL_CHECK(glGenTextures(1, &tex));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex));

	prog.uniform("tex", 0);
	prog.uniform("color", glm::vec3{1.0f, 0.475f, 0.0f});

	/* We require 1 byte alignment when uploading texture data */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* Clamping to edges is important to prevent artifacts when scaling */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Linear filtering usually looks best for text */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GL_CHECK(glBindVertexArray(vao));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));

	/* Loop through all characters */
	for (p = text.c_str(); *p; p++)
	{
		if (*p == '\n')
		{
			x = position.x;
			y -= lineHeight * sy;
			continue;
		}

		/* Try to load and render the character */
		FT_Error err = FT_Load_Char(face, *p, FT_LOAD_RENDER);
		if (err != 0)
		{
			cerr << "FT_Load_Char(" << *p << ")=" << err << endl;
			continue;
		}

		/* Upload the "bitmap", which contains an 8-bit grayscale image, as an alpha texture */
		//clog <<  << endl;
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex));
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g->bitmap.width, g->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer));

		// Calculate the vertex and texture coordinates
		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		float box[4][4] = {
			{x2,         -y2, 0.0f, 0.0f},
			{x2 + w,     -y2, 1.0f, 0.0f},
			{x2,     -y2 - h, 0.0f, 1.0f},
			{x2 + w, -y2 - h, 1.0f, 1.0f},
		};

		GL_CHECK(glEnableVertexAttribArray(0));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW));
		GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
		/* Draw the character on the screen */
		GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

		/* Advance the cursor to the start of the next character */
		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;

		// throw false;
	}

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDeleteTextures(1, &tex);
	glUseProgram(0);
}


}