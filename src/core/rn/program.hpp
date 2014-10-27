#ifndef RN_PROGRAM_HPP
#define RN_PROGRAM_HPP

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <list>
#include "../rn.hpp"
#include "types.hpp"
#include "../src/file.hpp"

namespace rn
{

// util::Sub reloadAll

class Program
{
public:
	typedef src::Stream Source;

	static std::list<Program *> collection;

	static void init();
	static void reloadLibs();
	static void reloadAll();

	static GLuint createShader(GLenum type, const GLchar *source, GLint sourceLength);
	static GLuint createShader(GLenum type, const std::string &source);
	static GLuint createShader(GLenum type, const std::vector<char> &source);
	static GLuint createProgram(const std::vector<GLuint> &shaders);

	GLuint id = 0;

	std::map<std::string, UniformValue> uniforms{};
	std::unique_ptr<Source> fragmentShader{nullptr};
	std::unique_ptr<Source> vertexShader{nullptr};

	std::string programName = "Unnamed program";

	Program();
	Program(std::string &&name);
	~Program();

	void load(const std::string &fragmentShader, const std::string &vertexShader);
	void load(Source *fragmentShader, Source *vertexShader);
	void load(std::unique_ptr<Source> &&fragmentShader, std::unique_ptr<Source> &&vertexShader);
	void reload();
	void reset();

	void use();
	void forgo();

	GLint getName(const std::string &name);
	UniformValue & getValue(const std::string &name);

	void var(GLint name, GLint value);
	void var(GLint name, GLuint value);
	void var(GLint name, GLfloat value);
	void var(GLint name, GLfloat x, GLfloat y);
	void var(GLint name, GLfloat x, GLfloat y, GLfloat z);
	void var(GLint name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void var(GLint name, const glm::vec2 &value);
	void var(GLint name, const glm::vec3 &value);
	void var(GLint name, const glm::vec4 &value);
	void var(GLint name, const glm::mat3 &value);
	void var(GLint name, const glm::mat4 &value);

	GLint var(const std::string &name, GLint value);
	GLint var(const std::string &name, GLuint value);
	GLint var(const std::string &name, GLfloat value);
	GLint var(const std::string &name, GLfloat x, GLfloat y);
	GLint var(const std::string &name, GLfloat x, GLfloat y, GLfloat z);
	GLint var(const std::string &name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	GLint var(const std::string &name, const glm::vec2 &value);
	GLint var(const std::string &name, const glm::vec3 &value);
	GLint var(const std::string &name, const glm::vec4 &value);
	GLint var(const std::string &name, const glm::mat3 &value);
	GLint var(const std::string &name, const glm::mat4 &value);

	GLint uniform(const std::string &name, GLint value);
	GLint uniform(const std::string &name, GLuint value);
	GLint uniform(const std::string &name, GLfloat value);
	GLint uniform(const std::string &name, GLfloat x, GLfloat y);
	GLint uniform(const std::string &name, GLfloat x, GLfloat y, GLfloat z);
	GLint uniform(const std::string &name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	GLint uniform(const std::string &name, const glm::vec2 &value);
	GLint uniform(const std::string &name, const glm::vec3 &value);
	GLint uniform(const std::string &name, const glm::vec4 &value);
	GLint uniform(const std::string &name, const glm::mat3 &value);
	GLint uniform(const std::string &name, const glm::mat4 &value);

};

} // rn

#endif