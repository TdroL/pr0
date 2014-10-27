#include "program.hpp"
#include "../rn.hpp"
#include "../rn/ext.hpp"
#include "../src/file.hpp"
#include "../util/initq.hpp"
#include <memory>
#include <string>
#include <iostream>

namespace rn
{

using namespace std;

typedef src::Stream Source;

list<Program *> Program::collection{};

struct Lib
{
	GLuint id = 0;
	unique_ptr<Source> source{nullptr};

	Lib() = default;
	Lib(Lib &&rhs)
		: id{move(rhs.id)}, source{move(rhs.source)}
	{
		rhs.id = 0;
		rhs.source.reset(nullptr);
	}
	explicit Lib(unique_ptr<Source> &&source)
		: source{move(source)}
	{}
};

vector<Lib> vertLibs{};
vector<Lib> fragLibs{};

void Program::init()
{
	fragLibs.emplace_back(src::file::stream("lib/normal.frag"));
	fragLibs.emplace_back(src::file::stream("lib/position.frag"));
	fragLibs.emplace_back(src::file::stream("lib/blur.frag"));
	fragLibs.emplace_back(src::file::stream("lib/vsm.frag"));
	fragLibs.emplace_back(src::file::stream("lib/esm.frag"));

	reloadLibs();
}

void Program::reloadLibs()
{
	for (auto &lib : vertLibs)
	{
		if ( ! lib.source)
		{
			throw string{"rn::Program::reloadLibs() - missing vertex shader library source"};
		}

		SRC_STREAM_OPEN(lib.source);

		if (lib.id)
		{
			RN_CHECK(glDeleteShader(lib.id));
		}

		lib.id = Program::createShader(GL_VERTEX_SHADER, lib.source->contents);
	}

	for (auto &lib : fragLibs)
	{
		if ( ! lib.source)
		{
			throw string{"rn::Program::reloadLibs() - missing fragment shader library source"};
		}

		SRC_STREAM_OPEN(lib.source);

		if (lib.id)
		{
			RN_CHECK(glDeleteShader(lib.id));
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
	RN_VALIDATE_PARAM(glCreateShader(type), rn::getEnumName(type));
	RN_CHECK(glShaderSource(shader, 1, &sourceCstr, &sourceLength));
	RN_CHECK(glCompileShader(shader));

	GLint status;
	RN_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));

	if (status == GL_FALSE)
	{
		GLint logLength;
		RN_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength));

		unique_ptr<GLchar[]> logCstr{new GLchar[logLength + 1]};
		logCstr[logLength] = '\0';
		RN_CHECK(glGetShaderInfoLog(shader, logLength, nullptr, logCstr.get()));

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

		throw string{"rn::Program::createShader() - compile failure in "} + typeVerbose + " shader:\n" + logCstr.get() + "\n" + string{sourceCstr, static_cast<size_t>(sourceLength)};
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
	RN_VALIDATE(glCreateProgram());

	for (auto shader : shaders)
	{
		RN_CHECK(glAttachShader(id, shader));
	}

	for (auto &lib : vertLibs)
	{
		if (lib.id)
		{
			RN_CHECK(glAttachShader(id, lib.id));
		}
	}

	for (auto &lib : fragLibs)
	{
		if (lib.id)
		{
			RN_CHECK(glAttachShader(id, lib.id));
		}
	}

	RN_CHECK(glLinkProgram(id));

	GLint status;
	RN_CHECK(glGetProgramiv(id, GL_LINK_STATUS, &status));

	if (status == GL_FALSE)
	{
		GLint logLength;
		RN_CHECK(glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength));

		unique_ptr<GLchar[]> logCstr{new GLchar[logLength + 1]};
		logCstr[logLength] = '\0';
		RN_CHECK(glGetProgramInfoLog(id, logLength, nullptr, logCstr.get()));

		throw string{"rn::Program::createProgram() - linker failure: "} + logCstr.get();
	}

	for (auto shader : shaders)
	{
		RN_CHECK(glDetachShader(id, shader));
		RN_CHECK(glDeleteShader(shader));
	}

	for (auto &lib : vertLibs)
	{
		if (lib.id)
		{
			RN_CHECK(glDetachShader(id, lib.id));
		}
	}

	for (auto &lib : fragLibs)
	{
		if (lib.id)
		{
			RN_CHECK(glDetachShader(id, lib.id));
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
		throw string{"rn::Program{" + programName + "}::reload() - missing fragment shader source"};
	}

	if ( ! vertexShader)
	{
		throw string{"rn::Program{" + programName + "}::reload() - missing vertex shader source"};
	}

	SRC_STREAM_OPEN(fragmentShader);
	SRC_STREAM_OPEN(vertexShader);

	vector<GLuint> shaders{
		Program::createShader(GL_FRAGMENT_SHADER, fragmentShader->contents),
		Program::createShader(GL_VERTEX_SHADER, vertexShader->contents)
	};

	id = Program::createProgram(shaders);

	if ( ! glIsProgram(id))
	{
		throw string{"rn::Program{" + programName + "}::reload() - could not create proper OpenGL shader program (\"" + fragmentShader->name() + "\", \"" + vertexShader->name() + "\")"};
	}

	for (auto &item : uniforms)
	{
		item.second.id = glGetUniformLocation(id, item.first.c_str());
		RN_VALIDATE(glGetUniformLocation(id, item.first.c_str()));

		typedef decltype(item.second.type) Type;

		switch (item.second.type)
		{
			case Type::uniform_int:
			{
				RN_CHECK(glProgramUniform1i(id, item.second.id, item.second.i));
				break;
			}
			case Type::uniform_uint:
			{
				RN_CHECK(glProgramUniform1ui(id, item.second.id, item.second.ui));
				break;
			}
			case Type::uniform_float:
			{
				RN_CHECK(glProgramUniform1f(id, item.second.id, item.second.f));
				break;
			}
			case Type::uniform_vec2:
			{
				RN_CHECK(glProgramUniform2fv(id, item.second.id, 1, glm::value_ptr(item.second.v2)));
				break;
			}
			case Type::uniform_vec3:
			{
				RN_CHECK(glProgramUniform3fv(id, item.second.id, 1, glm::value_ptr(item.second.v3)));
				break;
			}
			case Type::uniform_vec4:
			{
				RN_CHECK(glProgramUniform4fv(id, item.second.id, 1, glm::value_ptr(item.second.v4)));
				break;
			}
			case Type::uniform_mat3:
			{
				RN_CHECK(glProgramUniformMatrix3fv(id, item.second.id, 1, GL_FALSE, glm::value_ptr(item.second.m3)));
				break;
			}
			case Type::uniform_mat4:
			{
				RN_CHECK(glProgramUniformMatrix4fv(id, item.second.id, 1, GL_FALSE, glm::value_ptr(item.second.m4)));
				break;
			}
			default:
			{
				clog << "[warning] rn::Program::reload() - unknown uniform type " << item.second.type << " [" << item.first << "]" << endl;
			}
		}
	}
}

void Program::reset()
{
	if (id && glIsProgram(id))
	{
		RN_CHECK(glDeleteProgram(id));
	}

	id = 0;
}

void Program::use()
{
	#if defined(DEBUG) || defined(_DEBUG)
	if (id && ! glIsProgram(id))
	{
		clog << "[notice] rn::Program::use() - invalid program id [" << id << "] ("
			 << ((fragmentShader) ? fragmentShader->name() : string{"Unknown fragment shader"}) << ", "
			 << ((vertexShader) ? vertexShader->name() : string{"Unknown vertex shader"}) << ")" << endl;
	}
	#endif
	RN_CHECK(glUseProgram(id));
}

void Program::forgo()
{
	RN_CHECK(glUseProgram(0));
}

GLint Program::getName(const string &name)
{
	if (id)
	{
		GLint location = glGetUniformLocation(id, name.c_str());
		RN_VALIDATE(glGetUniformLocation(id, name.c_str()));

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

#include "program.var.inl"
#include "program.uniform.inl"

namespace
{
	const util::InitQAttacher attach(rn::initQ(), []
	{
		if ( ! rn::ext::ARB_separate_shader_objects) {
			throw string{"rn::Program initQ - rn::Program requires GL_ARB_separate_shader_objects"};
		}

		rn::Program::init();
	});
}

} // rn