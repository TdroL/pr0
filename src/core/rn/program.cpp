#include <pch.hpp>

#include "program.hpp"

#include "../rn.hpp"
#include "../rn/ext.hpp"
#include "../ngn.hpp"
#include "../ngn/fs.hpp"
#include "../src/file.hpp"
#include "../util/initq.hpp"
#include "../util/str.hpp"

#include <iostream>
#include <algorithm>
#include <regex>
#include <set>

namespace rn
{

using namespace std;

typedef src::Stream Source;

vector<Program *> Program::collection{};

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
	// fragLibs.emplace_back(src::file::stream("lib/csm.frag"));
	// fragLibs.emplace_back(src::file::stream("lib/normal.frag"));
	// fragLibs.emplace_back(src::file::stream("lib/position.frag"));
	// fragLibs.emplace_back(src::file::stream("lib/depth.frag"));
	// fragLibs.emplace_back(src::file::stream("lib/blur.frag"));
	// fragLibs.emplace_back(src::file::stream("lib/vsm.frag"));
	// fragLibs.emplace_back(src::file::stream("lib/esm.frag"));
	// fragLibs.emplace_back(src::file::stream("lib/util.frag"));

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

string Program::resolveIncludes(const GLchar *sourceCstr, GLint sourceLength)
{
	set<string> includedFiles{};

	regex rnIncludePattern{"[\f\r\t\v]*#pragma\\s+rn:\\s*include\\((.*)\\)[\f\r\t\v]*(\\n|$)"};
	regex versionPattern{"^(\\s*)(#version\\s+)"};
	smatch rnIncludeMatch;
	smatch versionMatch;

	string sourceResolved(sourceCstr, sourceLength);

	while (regex_search(sourceResolved, rnIncludeMatch, rnIncludePattern))
	{
		string fileName = ngn::fs::find(rnIncludeMatch[1].str());
		string fileContents{rnIncludeMatch[2].str()};

		if ( ! includedFiles.count(fileName))
		{
			fileContents = ngn::fs::contents(fileName) + rnIncludeMatch[2].str();

			if (regex_search(fileContents, versionMatch, versionPattern))
			{
				fileContents.replace(versionMatch.position(), versionMatch.length(), versionMatch[1].str() + "// "s + versionMatch[2].str());
			}

			includedFiles.insert(fileName);
		}

		sourceResolved.replace(rnIncludeMatch.position(), rnIncludeMatch.length(), fileContents);
	}

	return sourceResolved;
}

GLuint Program::createShader(GLenum type, const GLchar *sourceCstr, GLint sourceLength)
{
	GLuint shader = glCreateShader(type);
	RN_VALIDATE_PARAM(glCreateShader(type), rn::getEnumName(type));

	string sourceResolved = resolveIncludes(sourceCstr, sourceLength);

	sourceCstr = sourceResolved.data();
	sourceLength = sourceResolved.length();
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

		throw string{"rn::Program::createShader() - compile failure in "} + typeVerbose + " shader:\n" + util::str::prependLineNumbers(string{sourceCstr, static_cast<size_t>(sourceLength)}) + "\n" + logCstr.get();
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

GLuint Program::createProgram(const string &programName, const vector<GLuint> &shaders)
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

		throw string{"rn::Program::createProgram(" + programName + ") - linker failure: "} + logCstr.get();
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
	// Program::collection.remove(this);
	Program::collection.erase(remove(begin(Program::collection), end(Program::collection), this), end(Program::collection));
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
	double timer = ngn::time();

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

	hasCompileErrors = false;

	try
	{
		vector<GLuint> shaders{
			Program::createShader(GL_FRAGMENT_SHADER, fragmentShader->contents),
			Program::createShader(GL_VERTEX_SHADER, vertexShader->contents)
		};

		id = Program::createProgram(programName, shaders);
	}
	catch (...)
	{
		hasCompileErrors = true;
		throw;
	}

	if ( ! glIsProgram(id))
	{
		throw string{"rn::Program{" + programName + "}::reload() - could not create proper OpenGL shader program (\"" + fragmentShader->name() + "\", \"" + vertexShader->name() + "\")"};
	}

	for (auto &item : uniforms)
	{
		item.second.location = getUniformLocation(item.first);

		typedef decltype(item.second.type) Type;

		switch (item.second.type)
		{
			case Type::uniform_none:
			{
				// do nothing
				break;
			}
			case Type::uniform_int:
			{
				var(item.second.location, item.second.i);
				break;
			}
			case Type::uniform_uint:
			{
				var(item.second.location, item.second.ui);
				break;
			}
			case Type::uniform_float:
			{
				var(item.second.location, item.second.f);
				break;
			}
			case Type::uniform_vec2:
			{
				var(item.second.location, item.second.v2);
				break;
			}
			case Type::uniform_vec3:
			{
				var(item.second.location, item.second.v3);
				break;
			}
			case Type::uniform_vec4:
			{
				var(item.second.location, item.second.v4);
				break;
			}
			case Type::uniform_mat3:
			{
				var(item.second.location, item.second.m3);
				break;
			}
			case Type::uniform_mat4:
			{
				var(item.second.location, item.second.m4);
				break;
			}
			case Type::uniform_v_int:
			{
				var(item.second.location, item.second.vi.first, item.second.vi.second);
				break;
			}
			case Type::uniform_v_uint:
			{
				var(item.second.location, item.second.vui.first, item.second.vui.second);
				break;
			}
			case Type::uniform_v_float:
			{
				var(item.second.location, item.second.vf.first, item.second.vf.second);
				break;
			}
			case Type::uniform_v_vec2:
			{
				var(item.second.location, item.second.vv2.first, item.second.vv2.second);
				break;
			}
			case Type::uniform_v_vec3:
			{
				var(item.second.location, item.second.vv3.first, item.second.vv3.second);
				break;
			}
			case Type::uniform_v_vec4:
			{
				var(item.second.location, item.second.vv4.first, item.second.vv4.second);
				break;
			}
			case Type::uniform_v_mat3:
			{
				var(item.second.location, item.second.vm3.first, item.second.vm3.second);
				break;
			}
			case Type::uniform_v_mat4:
			{
				var(item.second.location, item.second.vm4.first, item.second.vm4.second);
				break;
			}
			default:
			{
				clog << "[warning] rn::Program{" << programName << "}::reload() - unknown uniform type " << item.second.type << " \"" << item.first << "\"" << endl;
			}
		}
	}

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [Program \"" << programName << "\" {frag:" << (fragmentShader ? "\"" + fragmentShader->name() + "\"" : "no source") << "; vert:" << (vertexShader ? "\"" + vertexShader->name() + "\"" : "no source") << "}:" << (ngn::time() - timer) << "s]" << endl;
		clog.unsetf(ios::floatfield);
	}
}

void Program::reset()
{
	if (id && glIsProgram(id))
	{
		RN_CHECK(glDeleteProgram(id));
	}

	id = 0;
	hasCompileErrors = false;
}

void Program::use()
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && id && ! glIsProgram(id))
	{
		clog << "[notice] rn::Program{" << programName << "}::use() - invalid program id \"" << id << "\" (" << (fragmentShader ? fragmentShader->name() : string{"Unknown fragment shader"}) << ", " << (vertexShader ? vertexShader->name() : string{"Unknown vertex shader"}) << ")" << endl;
	}
#endif

	RN_CHECK(glUseProgram(id));
}

void Program::forgo()
{
	RN_CHECK(glUseProgram(0));
}

GLint Program::getUniformLocation(const string &name)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && id && ! glIsProgram(id))
	{
		clog << "[notice] rn::Program{" << programName << "}::getUniformLocation(\"" << name << "\") - invalid program id \"" << id << "\" ("
		     << (fragmentShader ? fragmentShader->name() : string{"Unknown fragment shader"}) << ", "
		     << (vertexShader ? vertexShader->name() : string{"Unknown vertex shader"}) << ")" << endl;
	}
	else if ( ! hasCompileErrors && ! id)
	{
		clog << "[notice] rn::Program{" << programName << "}::getUniformLocation(\"" << name << "\") - program id is 0 ("
		     << (fragmentShader ? fragmentShader->name() : string{"Unknown fragment shader"}) << ", "
		     << (vertexShader ? vertexShader->name() : string{"Unknown vertex shader"}) << ")" << endl;
	}
#endif

	if (id)
	{
		GLint location = glGetUniformLocation(id, name.c_str());
		RN_VALIDATE_PARAM(glGetUniformLocation(id, name.c_str()), name);

		return location;
	}

	return 0;
}

UniformMeta & Program::getUniformMeta(const string &name)
{
	const auto &it = uniforms.find(name);

	if (it != end(uniforms))
	{
		return it->second;
	}

	GLint location = getUniformLocation(name);

	auto &uniformMeta = uniforms[name];
	uniformMeta.location = location;

	return uniformMeta;
}

void Program::var(GLint location, GLint value)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([GLint]) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform1i(id, location, value));
	}
}

void Program::var(GLint location, GLuint value)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([GLuint]) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform1ui(id, location, value));
	}
}

void Program::var(GLint location, GLfloat value)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([GLfloat]) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform1f(id, location, value));
	}
}

void Program::var(GLint location, const glm::vec2 &value)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([glm::vec2]) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform2fv(id, location, 1, glm::value_ptr(value)));
	}
}

void Program::var(GLint location, const glm::vec3 &value)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([glm::vec3]) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform3fv(id, location, 1, glm::value_ptr(value)));
	}
}

void Program::var(GLint location, const glm::vec4 &value)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([glm::vec4]) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform4fv(id, location, 1, glm::value_ptr(value)));
	}
}

void Program::var(GLint location, const glm::mat3 &value)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([glm::mat3]) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniformMatrix3fv(id, location, 1, GL_FALSE, glm::value_ptr(value)));
	}
}

void Program::var(GLint location, const glm::mat4 &value)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([glm::mat4]) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniformMatrix4fv(id, location, 1, GL_FALSE, glm::value_ptr(value)));
	}
}

void Program::var(GLint location, const GLint *value, GLsizei count)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([GLint], count) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform1iv(id, location, count, value));
	}
}

void Program::var(GLint location, const GLuint *value, GLsizei count)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([GLuint], count) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform1uiv(id, location, count, value));
	}
}

void Program::var(GLint location, const GLfloat *value, GLsizei count)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([GLfloat], count) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform1fv(id, location, count, value));
	}
}

void Program::var(GLint location, const glm::vec2 *value, GLsizei count)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([glm::vec2], count) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform2fv(id, location, count, reinterpret_cast<const GLfloat *>(value)));
	}
}

void Program::var(GLint location, const glm::vec3 *value, GLsizei count)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([glm::vec3], count) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform3fv(id, location, count, reinterpret_cast<const GLfloat *>(value)));
	}
}

void Program::var(GLint location, const glm::vec4 *value, GLsizei count)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([glm::vec4], count) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniform4fv(id, location, count, reinterpret_cast<const GLfloat *>(value)));
	}
}

void Program::var(GLint location, const glm::mat3 *value, GLsizei count)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([glm::mat3], count) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniformMatrix3fv(id, location, count, GL_FALSE, reinterpret_cast<const GLfloat *>(value)));
	}
}

void Program::var(GLint location, const glm::mat4 *value, GLsizei count)
{
#if defined(DEBUG)
	if ( ! hasCompileErrors && ! id)
	{
		clog << "rn::Program{" << programName << "}::var([glm::mat4], count) - program id is 0" << endl;
	}
#endif

	if (id)
	{
		RN_CHECK(glProgramUniformMatrix4fv(id, location, count, GL_FALSE, reinterpret_cast<const GLfloat *>(value)));
	}
}


GLint Program::var(const std::string &name, GLint value)
{
	GLint location = getUniformLocation(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, GLuint value)
{
	GLint location = getUniformLocation(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, GLfloat value)
{
	GLint location = getUniformLocation(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, const glm::vec2 &value)
{
	GLint location = getUniformLocation(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, const glm::vec3 &value)
{
	GLint location = getUniformLocation(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, const glm::vec4 &value)
{
	GLint location = getUniformLocation(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, const glm::mat3 &value)
{
	GLint location = getUniformLocation(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, const glm::mat4 &value)
{
	GLint location = getUniformLocation(name);
	var(location, value);
	return location;
}

GLint Program::var(const std::string &name, const GLint *value, GLsizei count)
{
	GLint location = getUniformLocation(name);
	var(location, value, count);
	return location;
}

GLint Program::var(const std::string &name, const GLuint *value, GLsizei count)
{
	GLint location = getUniformLocation(name);
	var(location, value, count);
	return location;
}

GLint Program::var(const std::string &name, const GLfloat *value, GLsizei count)
{
	GLint location = getUniformLocation(name);
	var(location, value, count);
	return location;
}

GLint Program::var(const std::string &name, const glm::vec2 *value, GLsizei count)
{
	GLint location = getUniformLocation(name);
	var(location, value, count);
	return location;
}

GLint Program::var(const std::string &name, const glm::vec3 *value, GLsizei count)
{
	GLint location = getUniformLocation(name);
	var(location, value, count);
	return location;
}

GLint Program::var(const std::string &name, const glm::vec4 *value, GLsizei count)
{
	GLint location = getUniformLocation(name);
	var(location, value, count);
	return location;
}

GLint Program::var(const std::string &name, const glm::mat3 *value, GLsizei count)
{
	GLint location = getUniformLocation(name);
	var(location, value, count);
	return location;
}

GLint Program::var(const std::string &name, const glm::mat4 *value, GLsizei count)
{
	GLint location = getUniformLocation(name);
	var(location, value, count);
	return location;
}

namespace
{
	const util::InitQAttacher attach(rn::initQ(), []
	{
		/*
		if ( ! rn::ext::ARB_direct_state_access && ! rn::ext::ARB_separate_shader_objects)
		{
			throw string{"rn::Program initQ - rn::Program requires GL_ARB_direct_state_access or GL_ARB_separate_shader_objects"};
		}
		*/

		rn::Program::init();
	});
}

} // rn