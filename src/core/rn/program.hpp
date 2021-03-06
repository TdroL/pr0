#pragma once

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <map>
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

	static std::vector<Program *> collection;

	static void init();
	static void reloadLibs();
	static void reloadAll();

	static std::string resolveIncludes(const GLchar *source, GLint length);
	static GLuint createShader(GLenum type, const GLchar *source, GLint length);
	static GLuint createShader(GLenum type, const std::string &source);
	static GLuint createShader(GLenum type, const std::vector<char> &source);
	static GLuint createProgram(const std::string &programName, const std::vector<GLuint> &shaders);

	GLuint id = 0;

	bool hasCompileErrors = false;

	std::map<std::string, UniformMeta> uniforms{};
	std::map<std::string, BufferMeta> uniformBuffers{};
	std::map<std::string, BufferMeta> shaderStorageBuffers{};

	std::unique_ptr<Source> fragmentShader{nullptr};
	std::unique_ptr<Source> vertexShader{nullptr};
	std::unique_ptr<Source> geometryShader{nullptr};

	std::string programName = "Unnamed program";

	Program();
	Program(std::string &&name);
	~Program();

	void load(const std::string &fragmentShader, const std::string &vertexShader);
	void load(const std::string &fragmentShader, const std::string &vertexShader, const std::string &geometryShader);
	void load(Source *fragmentShader, Source *vertexShader);
	void load(Source *fragmentShader, Source *vertexShader, Source *geometryShader);
	void load(std::unique_ptr<Source> &&fragmentShader, std::unique_ptr<Source> &&vertexShader);
	void load(std::unique_ptr<Source> &&fragmentShader, std::unique_ptr<Source> &&vertexShader, std::unique_ptr<Source> &&geometryShader);

	void reload();
	void reset();

	void use();
	void forgo();

	GLint getUniformLocation(const std::string &name);
	UniformMeta & getUniformMeta(const std::string &name);

	void var(GLint location, GLint value);
	void var(GLint location, GLuint value);
	void var(GLint location, GLfloat value);
	void var(GLint location, const glm::vec2 &value);
	void var(GLint location, const glm::vec3 &value);
	void var(GLint location, const glm::vec4 &value);
	void var(GLint location, const glm::mat3 &value);
	void var(GLint location, const glm::mat4 &value);

	void var(GLint location, const GLint *value, GLsizei count = 1);
	void var(GLint location, const GLuint *value, GLsizei count = 1);
	void var(GLint location, const GLfloat *value, GLsizei count = 1);
	void var(GLint location, const glm::vec2 *value, GLsizei count = 1);
	void var(GLint location, const glm::vec3 *value, GLsizei count = 1);
	void var(GLint location, const glm::vec4 *value, GLsizei count = 1);
	void var(GLint location, const glm::mat3 *value, GLsizei count = 1);
	void var(GLint location, const glm::mat4 *value, GLsizei count = 1);

	GLint var(const std::string &name, GLint value);
	GLint var(const std::string &name, GLuint value);
	GLint var(const std::string &name, GLfloat value);
	GLint var(const std::string &name, const glm::vec2 &value);
	GLint var(const std::string &name, const glm::vec3 &value);
	GLint var(const std::string &name, const glm::vec4 &value);
	GLint var(const std::string &name, const glm::mat3 &value);
	GLint var(const std::string &name, const glm::mat4 &value);

	GLint var(const std::string &name, const GLint *value, GLsizei count = 1);
	GLint var(const std::string &name, const GLuint *value, GLsizei count = 1);
	GLint var(const std::string &name, const GLfloat *value, GLsizei count = 1);
	GLint var(const std::string &name, const glm::vec2 *value, GLsizei count = 1);
	GLint var(const std::string &name, const glm::vec3 *value, GLsizei count = 1);
	GLint var(const std::string &name, const glm::vec4 *value, GLsizei count = 1);
	GLint var(const std::string &name, const glm::mat3 *value, GLsizei count = 1);
	GLint var(const std::string &name, const glm::mat4 *value, GLsizei count = 1);

	template<typename T>
	GLint uniform(const std::string &name, T value)
	{
		UniformMeta &uniformMeta = getUniformMeta(name);

		var(uniformMeta.location, uniformMeta.set(value));

		return uniformMeta.location;
	}

	template<typename T>
	GLint uniform(const std::string &name, std::unique_ptr<T[]> &&value, GLsizei count)
	{
		UniformMeta &uniformMeta = getUniformMeta(name);

		if (value)
		{
			var(uniformMeta.location, uniformMeta.set(value.release(), count), count);
		}

		return uniformMeta.location;
	}

};

} // rn