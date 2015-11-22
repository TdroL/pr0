#ifndef RN_HPP
#define RN_HPP

#include <string>
#include <iostream>

#if defined(NGN_USE_GLEW)
	#include <GL/glew.h>
#elif defined(NGN_USE_GL3W)
	#include <GL/gl3w.h>
#elif defined(NGN_USE_GLLOADGEN)
	#include <GL/gl_core_4_4.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
// #include <glm/gtc/matrix_transform.hpp>

#include "util/initq.hpp"
#include "util.hpp"

namespace rn
{

extern const char *lastGLCall;
extern bool logGLCalls;

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

struct Default
{
	static constexpr GLenum cullFace = GL_BACK;
	static constexpr GLenum frontFace = GL_CCW;
	static constexpr GLenum depthMask = GL_TRUE;
	static constexpr GLenum clipControlOrigin = GL_LOWER_LEFT;
	static constexpr GLenum clipControlDepth = GL_ZERO_TO_ONE;
	static constexpr GLenum depthFunc = GL_GEQUAL;
	static constexpr GLenum blendFuncSFactor = GL_SRC_ALPHA;
	static constexpr GLenum blendFuncDFactor = GL_ONE_MINUS_SRC_ALPHA;
};

template<typename T>
struct TypeLimit
{};

template <>
struct TypeLimit<GLubyte>
{
	static constexpr GLubyte min = 0U;
	static constexpr GLubyte max = 255U;
};

template <>
struct TypeLimit<GLbyte>
{
	static constexpr GLbyte min = -128;
	static constexpr GLbyte max = 127;
};

template <>
struct TypeLimit<GLushort>
{
	static constexpr GLushort min = 0U;
	static constexpr GLushort max = 65535U;
};

template <>
struct TypeLimit<GLshort>
{
	static constexpr GLshort min = -32768;
	static constexpr GLshort max = 32767;
};

template <>
struct TypeLimit<GLuint>
{
	static constexpr GLuint min = 0U;
	static constexpr GLuint max = 4294967295U;
};

template <>
struct TypeLimit<GLint>
{
	static constexpr GLint min = -2147483648;
	static constexpr GLint max = 2147483647;
};

template <>
struct TypeLimit<GLfloat>
{
	static constexpr GLfloat min = -340282346638528859811704183484516925440.0f;
	static constexpr GLfloat max = -340282346638528859811704183484516925440.0f;
};

template <>
struct TypeLimit<GLdouble>
{
	static constexpr GLdouble min = 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0;
	static constexpr GLdouble max = -179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0;
};

extern Stats stats;
extern Status status;

util::InitQ & initQ();

void init();
void resetAllContextRelated();
void reload();
void reloadAll();
void reloadSoftAll();

std::string getEnumName(GLenum value);
std::string getBasicInfo();
std::string getExtensionsInfo();

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

#define RN_CHECK_LOG_ERROR(name, error) { std::cerr << "[GL error]:" << __FILE__ << ":" << __LINE__ << ": " << #name << " = " << rn::getEnumName(error) << " (err = 0x" << std::hex << error << std::dec << ")" << std::endl << std::flush; }

#define RN_CHECK_LOG_ERROR_PARAM(name, error, param) { std::cerr << "[GL error]:" << __FILE__ << ":" << __LINE__ << ": " << #name << " = " << rn::getEnumName(error) << " (err = 0x" << std::hex << error << std::dec << ") -- " << param << std::endl << std::flush; }

#if defined(DEBUG)

	#define RN_SAVE_CALL(fn) do \
	{ \
		rn::lastGLCall = "[" __FILE__  ":" UTIL_STRINGIFY(__LINE__) "] " #fn; \
	} \
	while (0)

	#define RN_VALIDATE(fn) do \
	{ \
		if (rn::logGLCalls) \
		{ \
			std::clog << "[GL call]:" << __FILE__ << ":" << __LINE__ << ": " << #fn << std::endl; \
		} \
		GLenum err = glGetError(); \
		if (err != GL_NO_ERROR) \
		{ \
			do \
			{ \
				RN_CHECK_LOG_ERROR(fn, err); \
				err = glGetError(); \
			} \
			while (err != GL_NO_ERROR); \
			exit(1); \
		} \
	} \
	while(0)

	#define RN_VALIDATE_PARAM(fn, param) do \
	{ \
		if (rn::logGLCalls) \
		{ \
			std::clog << "[GL call]:" << __FILE__ << ":" << __LINE__ << ": " << #fn << std::endl; \
		} \
		GLenum err = glGetError(); \
		if (err != GL_NO_ERROR) \
		{ \
			do \
			{ \
				RN_CHECK_LOG_ERROR_PARAM(fn, err, param); \
				err = glGetError(); \
			} \
			while (err != GL_NO_ERROR); \
			exit(1); \
		} \
	} \
	while(0)

	#define RN_CHECK(fn) do \
	{ \
		RN_SAVE_CALL(fn); \
		fn; \
		RN_VALIDATE(fn); \
	} \
	while (0)

	#define RN_CHECK_PARAM(fn, param) do \
	{ \
		RN_SAVE_CALL(fn); \
		fn; \
		RN_VALIDATE_PARAM(fn, param); \
	} \
	while (0)

#else

	#define RN_SAVE_CALL(fn)             do { } while(0)
	#define RN_VALIDATE(fn)              do { } while(0)
	#define RN_VALIDATE_PARAM(fn, param) do { } while(0)
	#define RN_CHECK(fn)                 do { fn; } while(0)
	#define RN_CHECK_PARAM(fn, param)    do { fn; } while(0)

#endif

class EnableScoper
{
public:
	GLenum name;
	GLboolean wasEnabled;

	EnableScoper(GLenum name)
		: name{name}, wasEnabled{glIsEnabled(name)}
	{
		RN_VALIDATE_PARAM(glIsEnabled(name), rn::getEnumName(name));

		RN_CHECK(glEnable(name));
	}
	~EnableScoper()
	{
		if ( ! wasEnabled)
		{
			RN_CHECK(glDisable(name));
		}
	}
};

class DisableScoper
{
public:
	GLenum name;
	GLboolean wasEnabled;

	DisableScoper(GLenum name)
		: name{name}, wasEnabled{glIsEnabled(name)}
	{
		RN_VALIDATE_PARAM(glIsEnabled(name), rn::getEnumName(name));

		RN_CHECK(glDisable(name));
	}
	~DisableScoper()
	{
		if (wasEnabled)
		{
			RN_CHECK(glEnable(name));
		}
	}
};

#define RN_SCOPE_ENABLE(name) rn::EnableScoper UTIL_CONCAT2(glEnableScoper, __COUNTER__)(name)
#define RN_SCOPE_DISABLE(name) rn::DisableScoper UTIL_CONCAT2(glDisableScoper, __COUNTER__)(name)

} // rn

#endif