#ifndef GL_HPP
#define GL_HPP

#define GLM_SWIZZLE

#include <string>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util/initq.hpp"
#include "util.hpp"

namespace gl
{

extern util::InitQ initQ;

enum Status
{
	uninited = 0,
	inited = 1,
};

struct Stats
{
	size_t triangles;
	size_t meshes;

	void reset()
	{
		triangles = 0;
		meshes = 0;
	}
};

extern Stats stats;
extern Status status;

void init();
void reload();
void reloadAll();
void reloadSoftAll();

std::string getEnumName(GLenum value);
std::string getBasicInfo(std::string prefix = "");
std::string getExtensionsInfo(std::string prefix = "");

void get(const GLenum name, GLboolean &value);
void get(const GLenum name, GLint &value);
void get(const GLenum name, GLuint &value);
void get(const GLenum name, GLint64 &value);
void get(const GLenum name, GLfloat &value);
void get(const GLenum name, GLdouble &value);
void get(const GLenum name, glm::vec2 &value);
void get(const GLenum name, glm::vec3 &value);
void get(const GLenum name, glm::vec4 &value);

void get(const GLenum name, GLboolean *value);
void get(const GLenum name, GLint *value);
void get(const GLenum name, GLuint *value);
void get(const GLenum name, GLint64 *value);
void get(const GLenum name, GLfloat *value);
void get(const GLenum name, GLdouble *value);
void get(const GLenum name, glm::vec2 *value);
void get(const GLenum name, glm::vec3 *value);
void get(const GLenum name, glm::vec4 *value);

void get(const GLenum name, GLboolean &value, GLuint i);
void get(const GLenum name, GLint &value, GLuint i);
void get(const GLenum name, GLuint &value, GLuint i);
void get(const GLenum name, GLint64 &value, GLuint i);
void get(const GLenum name, GLfloat &value, GLuint i);
void get(const GLenum name, GLdouble &value, GLuint i);

void get(const GLenum name, GLboolean *value, GLuint i);
void get(const GLenum name, GLint *value, GLuint i);
void get(const GLenum name, GLuint *value, GLuint i);
void get(const GLenum name, GLint64 *value, GLuint i);
void get(const GLenum name, GLfloat *value, GLuint i);
void get(const GLenum name, GLdouble *value, GLuint i);

void flushErrors();

#define GL_CHECK_LOG_ERROR(name, error) { std::cerr << "[GL error]:" << __FILE__ << ":" << __LINE__ << ": " << #name << " = " << gl::getEnumName(error) << " (err = 0x" << std::hex << error << std::dec << ")" << std::endl << std::flush; }

#define GL_CHECK_LOG_ERROR_PARAM(name, error, param) { std::cerr << "[GL error]:" << __FILE__ << ":" << __LINE__ << ": " << #name << " = " << gl::getEnumName(error) << " (err = 0x" << std::hex << error << std::dec << ") -- " << param << std::endl << std::flush; }

#define GL_VALIDATE(fn) { GLenum err = glGetError(); if (err != GL_NO_ERROR) { do { GL_CHECK_LOG_ERROR(fn, err); err = glGetError(); } while (err != GL_NO_ERROR); exit(1); } }

#define GL_VALIDATE_PARAM(fn, param) { GLenum err = glGetError(); if (err != GL_NO_ERROR) { do { GL_CHECK_LOG_ERROR_PARAM(fn, err, param); err = glGetError(); } while (err != GL_NO_ERROR); exit(1); } }

#define GL_CHECK(fn) { fn; GL_VALIDATE(fn); }

#define GL_CHECK_PARAM(fn, param) { fn; GL_VALIDATE_PARAM(fn, param); }

class EnableScoper
{
public:
	GLenum name;

	EnableScoper(GLenum name) : name{name} { GL_CHECK(glEnable(name)); }
	~EnableScoper() { GL_CHECK(glDisable(name)); }
};

class DisableScoper
{
public:
	GLenum name;

	DisableScoper(GLenum name) : name{name} { GL_CHECK(glDisable(name)); }
	~DisableScoper() { GL_CHECK(glEnable(name)); }
};

#define GL_SCOPE_ENABLE(name) gl::EnableScoper UTIL_CONCAT2(glEnableScoper, __COUNTER__)(name)
#define GL_SCOPE_DISABLE(name) gl::DisableScoper UTIL_CONCAT2(glDisableScoper, __COUNTER__)(name)

} // gl

#endif