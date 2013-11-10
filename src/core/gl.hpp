#ifndef GL_HPP
#define GL_HPP

#include <string>
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gl
{

enum Status
{
	uninited = 0,
	inited = 1,
};

extern Status status;

void init();
void reload();

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

#define GL_CHECK_LOG_ERROR(name, error) { std::cerr << "[GL error]:" << __FILE__ << ":" << __LINE__ << ": " << #name << " = " << gl::getEnumName(error) << " (err=" << error << ")" << std::endl; }

#define GL_CHECK_LOG_ERROR_PARAM(name, error, param) { std::cerr << "[GL error]:" << __FILE__ << ":" << __LINE__ << ": " << #name << " = " << gl::getEnumName(error) << " (err=" << error << ") -- " << param << std::endl; }

#define GL_VALIDATE(fn) { GLenum err = glGetError(); if (err != GL_NO_ERROR) { GL_CHECK_LOG_ERROR(fn, err); exit(1); } }

#define GL_VALIDATE_PARAM(fn, param) { GLenum err = glGetError(); if (err != GL_NO_ERROR) { GL_CHECK_LOG_ERROR_PARAM(fn, err, param); exit(1); } }

#define GL_CHECK(fn) { fn; GL_VALIDATE(fn); }

#define GL_CHECK_PARAM(fn, param) { fn; GL_VALIDATE_PARAM(fn, param); }

} // gl

#endif