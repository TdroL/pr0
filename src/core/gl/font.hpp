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

class Font
{
public:
	typedef src::Stream Source;

	static std::list<Font *> collection;
	static void reloadAll();
	static void init();
	static void deinit();

	std::unique_ptr<Source> source{nullptr};

	FT_Face face{nullptr};
	GLuint tex = 0;
	GLuint vao = 0;
	GLuint vbo = 0;

	glm::vec2 position{-1.f, 1.f};
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