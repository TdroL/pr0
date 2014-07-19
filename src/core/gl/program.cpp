#include "program.hpp"
#include "../gl.hpp"
#include "../src/file.hpp"
#include "../util/initq.hpp"
#include <memory>
#include <iostream>

namespace gl
{

using namespace std;

typedef src::Stream Source;

list<Program *> Program::collection{};

struct Lib
{
	GLuint id = 0;
	unique_ptr<Source> source{nullptr};

	Lib() = default;
	explicit Lib(unique_ptr<Source> &&source)
		: source{move(source)}
	{}
};

vector<Lib> vectLibs{};
vector<Lib> fragLibs{};

void Program::init()
{
	vectLibs.push_back(Lib{src::file::stream("lib/normal.vert")});

	fragLibs.push_back(Lib{src::file::stream("lib/normal.frag")});
	fragLibs.push_back(Lib{src::file::stream("lib/position.frag")});
	fragLibs.push_back(Lib{src::file::stream("lib/vsm.frag")});

	reloadLibs();
}

void Program::reloadLibs()
{
	for (auto &lib : vectLibs)
	{
		SRC_STREAM_USE(*(lib.source));

		if (lib.id)
		{
			GL_CHECK(glDeleteShader(lib.id));
		}

		lib.id = Program::createShader(GL_FRAGMENT_SHADER, lib.source->contents);
	}

	for (auto &lib : fragLibs)
	{
		SRC_STREAM_USE(*(lib.source));

		if (lib.id)
		{
			GL_CHECK(glDeleteShader(lib.id));
		}

		lib.id = Program::createShader(GL_FRAGMENT_SHADER, lib.source->contents);
	}
}

void Program::reloadAll()
{
	reloadLibs();

	for (Program *prog : Program::collection)
	{
		try
		{
			prog->reload();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

GLuint Program::createShader(GLenum type, const GLchar *sourceCstr, GLint sourceLength)
{
	GLuint shader = glCreateShader(type);
	GL_VALIDATE_PARAM(glCreateShader(type), gl::getEnumName(type));
	GL_CHECK(glShaderSource(shader, 1, &sourceCstr, &sourceLength));
	GL_CHECK(glCompileShader(shader));

	GLint status;
	GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));

	if (status == GL_FALSE)
	{
		GLint logLength;
		GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength));

		unique_ptr<GLchar[]> logCstr{new GLchar[logLength + 1]};
		logCstr[logLength] = '\0';
		GL_CHECK(glGetShaderInfoLog(shader, logLength, nullptr, logCstr.get()));

		string typeVerbose;
		switch(type)
		{
		case GL_VERTEX_SHADER:
			typeVerbose = "vertex";
			break;
		case GL_GEOMETRY_SHADER:
			typeVerbose = "geometry";
			break;
		case GL_FRAGMENT_SHADER:
			typeVerbose = "fragment";
			break;
		}

		throw string{"gl::Program::createShader() - compile failure in "} + typeVerbose + " shader:\n" + logCstr.get() + "\n" + string{sourceCstr, static_cast<size_t>(sourceLength)};
	}

	return shader;
}

GLuint Program::createShader(GLenum type, const std::string &source)
{
	const GLchar *sourceCstr = reinterpret_cast<const GLchar*>(source.c_str());
	const GLint sourceLength = source.size() * sizeof(source[0]);

	return Program::createShader(type, sourceCstr, sourceLength);
}
GLuint Program::createShader(GLenum type, const std::vector<char> &source)
{
	const GLchar *sourceCstr = reinterpret_cast<const GLchar*>(&source[0]);
	const GLint sourceLength = source.size() * sizeof(source[0]);

	return Program::createShader(type, sourceCstr, sourceLength);
}

GLuint Program::createProgram(const vector<GLuint> &shaders)
{
	GLuint id = glCreateProgram();
	GL_VALIDATE(glCreateProgram());

	for (auto shader : shaders)
	{
		GL_CHECK(glAttachShader(id, shader));
	}

	for (auto &lib : vectLibs)
	{
		if (lib.id)
		{
			GL_CHECK(glAttachShader(id, lib.id));
		}
	}

	for (auto &lib : fragLibs)
	{
		if (lib.id)
		{
			GL_CHECK(glAttachShader(id, lib.id));
		}
	}

	GL_CHECK(glLinkProgram(id));

	GLint status;
	GL_CHECK(glGetProgramiv(id, GL_LINK_STATUS, &status));

	if (status == GL_FALSE)
	{
		GLint logLength;
		GL_CHECK(glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength));

		unique_ptr<GLchar[]> logCstr{new GLchar[logLength + 1]};
		logCstr[logLength] = '\0';
		GL_CHECK(glGetProgramInfoLog(id, logLength, nullptr, logCstr.get()));

		throw string{"gl::Program::createProgram() - linker failure: "} + logCstr.get();
	}

	for (auto shader : shaders)
	{
		GL_CHECK(glDetachShader(id, shader));
		GL_CHECK(glDeleteShader(shader));
	}

	for (auto &lib : vectLibs)
	{
		if (lib.id)
		{
			GL_CHECK(glDetachShader(id, lib.id));
		}
	}

	for (auto &lib : fragLibs)
	{
		if (lib.id)
		{
			GL_CHECK(glDetachShader(id, lib.id));
		}
	}

	return id;
}

Program::Program()
{
	Program::collection.push_back(this);
}

Program::Program(string &&name)
	: Program{}
{
	programName = move(name);
}

Program::~Program()
{
	Program::collection.remove(this);
}

void Program::load(const string &fragmentShader, const string &vertexShader)
{
	this->fragmentShader = src::file::stream(fragmentShader);
	this->vertexShader = src::file::stream(vertexShader);

	reload();
}

void Program::load(Source *fragmentShader, Source *vertexShader)
{
	this->fragmentShader.reset(fragmentShader);
	this->vertexShader.reset(vertexShader);

	reload();
}

void Program::load(unique_ptr<Source> &&fragmentShader, unique_ptr<Source> &&vertexShader)
{
	this->fragmentShader = move(fragmentShader);
	this->vertexShader = move(vertexShader);

	reload();
}

void Program::reload()
{
	reset();

	if ( ! fragmentShader)
	{
		throw string{"gl::Program{" + programName + "}::reload() - missing fragment shader source"};
	}

	if ( ! vertexShader)
	{
		throw string{"gl::Program{" + programName + "}::reload() - missing vertex shader source"};
	}

	SRC_STREAM_USE(*fragmentShader);
	SRC_STREAM_USE(*vertexShader);

	vector<GLuint> shaders{
		Program::createShader(GL_FRAGMENT_SHADER, fragmentShader->contents),
		Program::createShader(GL_VERTEX_SHADER, vertexShader->contents)
	};

	id = Program::createProgram(shaders);

	if ( ! glIsProgram(id))
	{
		throw string{"gl::Program{" + programName + "}::reload() - could not create proper OpenGL shader program (\"" + fragmentShader->name() + "\", \"" + vertexShader->name() + "\")"};
	}

	for (auto &item : uniforms)
	{
		item.second.id = glGetUniformLocation(id, item.first.c_str());
		GL_VALIDATE(glGetUniformLocation(id, item.first.c_str()));

		typedef decltype(item.second.type) Type;

		switch (item.second.type)
		{
			case Type::uniform_int:
			{
				GL_CHECK(glProgramUniform1i(id, item.second.id, item.second.i));
				break;
			}
			case Type::uniform_uint:
			{
				GL_CHECK(glProgramUniform1ui(id, item.second.id, item.second.ui));
				break;
			}
			case Type::uniform_float:
			{
				GL_CHECK(glProgramUniform1f(id, item.second.id, item.second.f));
				break;
			}
			case Type::uniform_vec2:
			{
				GL_CHECK(glProgramUniform2fv(id, item.second.id, 1, glm::value_ptr(item.second.v2)));
				break;
			}
			case Type::uniform_vec3:
			{
				GL_CHECK(glProgramUniform3fv(id, item.second.id, 1, glm::value_ptr(item.second.v3)));
				break;
			}
			case Type::uniform_vec4:
			{
				GL_CHECK(glProgramUniform4fv(id, item.second.id, 1, glm::value_ptr(item.second.v4)));
				break;
			}
			case Type::uniform_mat3:
			{
				GL_CHECK(glProgramUniformMatrix3fv(id, item.second.id, 1, GL_FALSE, glm::value_ptr(item.second.m3)));
				break;
			}
			case Type::uniform_mat4:
			{
				GL_CHECK(glProgramUniformMatrix4fv(id, item.second.id, 1, GL_FALSE, glm::value_ptr(item.second.m4)));
				break;
			}
			default:
			{
				clog << "[warning] gl::Program::reload() - unknown uniform type " << item.second.type << " [" << item.first << "]" << endl;
			}
		}
	}
}

void Program::reset()
{
	if (id && glIsProgram(id))
	{
		GL_CHECK(glDeleteProgram(id));
	}

	id = 0;
}

void Program::use()
{
	#if defined(DEBUG) || defined(_DEBUG)
	if (id && ! glIsProgram(id))
	{
		clog << "[notice] gl::Program::use() - invalid program id [" << id << "] ("
			 << ((fragmentShader) ? fragmentShader->name() : string{"Unknown fragment shader"}) << ", "
			 << ((vertexShader) ? vertexShader->name() : string{"Unknown vertex shader"}) << ")" << endl;
	}
	#endif
	GL_CHECK(glUseProgram(id));
}

void Program::release()
{
	GL_CHECK(glUseProgram(0));
}

GLint Program::getName(const string &name)
{
	if (id)
	{
		GLint location = glGetUniformLocation(id, name.c_str());
		GL_VALIDATE(glGetUniformLocation(id, name.c_str()));

		return location;
	}

	return 0;
}

UniformValue & Program::getValue(const string &name)
{
	const auto &it = uniforms.find(name);

	if (it != end(uniforms))
	{
		return it->second;
	}

	GLint location = getName(name);

	auto &uniformValue = uniforms[name];
	uniformValue.id = location;

	return uniformValue;
}

#include "program.cpp.var"
#include "program.cpp.uniform"

namespace
{
	const util::InitQAttacher attach{gl::initQ, []
	{
		gl::Program::init();
	}};
}

} // gl