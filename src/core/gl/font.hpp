#ifndef GL_FONT_HPP
#define GL_FONT_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "../src.hpp"

namespace gl
{

struct FontChar
{
	float ax = 0.0f; // advance.x
	float ay = 0.0f; // advance.y

	float bw = 0.0f; // bitmap.width;
	float bh = 0.0f; // bitmap.rows;

	float bl = 0.0f; // bitmap_left;
	float bt = 0.0f; // bitmap_top;

	float tx = 0.0f; // x offset of glyph in texture coordinates
	float ty = 0.0f; // y offset of glyph in texture coordinates
};

class Font
{
public:
	typedef src::Stream Source;

	static std::list<Font *> collection;
	static void reloadAll();
	static void init();
	static void deinit();

	std::unique_ptr<Source> source{nullptr};

	float atlas_width = 0.0f;
	float atlas_height = 0.0f;

	FT_Face face{nullptr};
	GLuint tex = 0;
	GLuint vao = 0;
	GLuint vbo = 0;

	std::vector<FontChar> chars{};

	glm::vec2 position{-1.0f, 1.0f};
	glm::vec3 color{1.0f, 0.475f, 0.0f};
	int fontSize = 16;
	int lineHeight = 20;

	Font();
	~Font();

	Font(const Font&) = delete;
	Font(Font&&) = delete;
	Font & operator=(const Font&) = delete;
	Font & operator=(Font&&) = delete;

	void load(std::string &&source);
	void load(Source *source);
	void load(std::unique_ptr<Source> &&source);
	void reload();
	void reset();

	void render(const std::string &text);
};

}

#endif