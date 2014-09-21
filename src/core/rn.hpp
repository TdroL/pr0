#ifndef RN_HPP
#define RN_HPP

#define GLM_SWIZZLE

#include <string>
#include <iostream>

#ifdef NGN_USE_GLEW
	#include <GL/glew.h>
#else
	#include <GL/gl3w.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "util/initq.hpp"
#include "util.hpp"

namespace rn
{

extern const char *lastGLCall;

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

util::InitQ & initQ();

void init();
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

class Ext {
public:
	std::string name;

	explicit Ext(std::string &&name)
		: name{std::move(name)} {}

	bool test();

	operator bool();
};

extern Ext ext_ARB_arrays_of_arrays;
extern Ext ext_ARB_base_instance;
extern Ext ext_ARB_blend_func_extended;
extern Ext ext_ARB_cl_event;
extern Ext ext_ARB_clear_buffer_object;
extern Ext ext_ARB_compressed_texture_pixel_storage;
extern Ext ext_ARB_compute_shader;
extern Ext ext_ARB_conservative_depth;
extern Ext ext_ARB_copy_buffer;
extern Ext ext_ARB_copy_image;
extern Ext ext_ARB_debug_output;
extern Ext ext_ARB_depth_buffer_float;
extern Ext ext_ARB_depth_clamp;
extern Ext ext_ARB_draw_buffers_blend;
extern Ext ext_ARB_draw_elements_base_vertex;
extern Ext ext_ARB_draw_indirect;
extern Ext ext_ARB_ES2_compatibility;
extern Ext ext_ARB_ES3_compatibility;
extern Ext ext_ARB_explicit_attrib_location;
extern Ext ext_ARB_explicit_uniform_location;
extern Ext ext_ARB_fragment_coord_conventions;
extern Ext ext_ARB_fragment_layer_viewport;
extern Ext ext_ARB_framebuffer_no_attachments;
extern Ext ext_ARB_framebuffer_object;
extern Ext ext_ARB_framebuffer_sRGB;
extern Ext ext_ARB_get_program_binary;
extern Ext ext_ARB_gpu_shader5;
extern Ext ext_ARB_gpu_shader_fp64;
extern Ext ext_ARB_half_float_pixel;
extern Ext ext_ARB_half_float_vertex;
extern Ext ext_ARB_imaging;
extern Ext ext_ARB_internalformat_query;
extern Ext ext_ARB_internalformat_query2;
extern Ext ext_ARB_invalidate_subdata;
extern Ext ext_ARB_map_buffer_alignment;
extern Ext ext_ARB_map_buffer_range;
extern Ext ext_ARB_multi_draw_indirect;
extern Ext ext_ARB_occlusion_query2;
extern Ext ext_ARB_program_interface_query;
extern Ext ext_ARB_provoking_vertex;
extern Ext ext_ARB_robust_buffer_access_behavior;
extern Ext ext_ARB_robustness;
extern Ext ext_ARB_robustness_isolation;
extern Ext ext_ARB_sample_shading;
extern Ext ext_ARB_sampler_objects;
extern Ext ext_ARB_seamless_cube_map;
extern Ext ext_ARB_separate_shader_objects;
extern Ext ext_ARB_shader_atomic_counters;
extern Ext ext_ARB_shader_bit_encoding;
extern Ext ext_ARB_shader_image_load_store;
extern Ext ext_ARB_shader_image_size;
extern Ext ext_ARB_shader_objects;
extern Ext ext_ARB_shader_precision;
extern Ext ext_ARB_shader_stencil_export;
extern Ext ext_ARB_shader_storage_buffer_object;
extern Ext ext_ARB_shader_subroutine;
extern Ext ext_ARB_shading_language_420pack;
extern Ext ext_ARB_shading_language_include;
extern Ext ext_ARB_shading_language_packing;
extern Ext ext_ARB_stencil_texturing;
extern Ext ext_ARB_sync;
extern Ext ext_ARB_tessellation_shader;
extern Ext ext_ARB_texture_buffer_object_rgb32;
extern Ext ext_ARB_texture_buffer_range;
extern Ext ext_ARB_texture_compression_bptc;
extern Ext ext_ARB_texture_compression_rgtc;
extern Ext ext_ARB_texture_cube_map_array;
extern Ext ext_ARB_texture_gather;
extern Ext ext_ARB_texture_multisample;
extern Ext ext_ARB_texture_query_levels;
extern Ext ext_ARB_texture_query_lod;
extern Ext ext_ARB_texture_rg;
extern Ext ext_ARB_texture_rgb10_a2ui;
extern Ext ext_ARB_texture_storage;
extern Ext ext_ARB_texture_storage_multisample;
extern Ext ext_ARB_texture_swizzle;
extern Ext ext_ARB_texture_view;
extern Ext ext_ARB_timer_query;
extern Ext ext_ARB_transform_feedback2;
extern Ext ext_ARB_transform_feedback3;
extern Ext ext_ARB_transform_feedback_instanced;
extern Ext ext_ARB_uniform_buffer_object;
extern Ext ext_ARB_vertex_array_bgra;
extern Ext ext_ARB_vertex_array_object;
extern Ext ext_ARB_vertex_attrib_64bit;
extern Ext ext_ARB_vertex_attrib_binding;
extern Ext ext_ARB_vertex_buffer_object;
extern Ext ext_ARB_vertex_type_2_10_10_10_rev;
extern Ext ext_ARB_viewport_array;
extern Ext ext_KHR_debug;
extern Ext ext_KHR_texture_compression_astc_ldr;

#define RN_CHECK_LOG_ERROR(name, error) { std::cerr << "[GL error]:" << __FILE__ << ":" << __LINE__ << ": " << #name << " = " << rn::getEnumName(error) << " (err = 0x" << std::hex << error << std::dec << ")" << std::endl << std::flush; }

#define RN_CHECK_LOG_ERROR_PARAM(name, error, param) { std::cerr << "[GL error]:" << __FILE__ << ":" << __LINE__ << ": " << #name << " = " << rn::getEnumName(error) << " (err = 0x" << std::hex << error << std::dec << ") -- " << param << std::endl << std::flush; }

#if defined(DEBUG) || defined (_DEBUG)

	#define RN_SAVE_CALL(fn) do { \
		rn::lastGLCall = "[" __FILE__  ":" UTIL_STRINGIFY(__LINE__) "] " #fn; \
	} while (0)

	#define RN_VALIDATE(fn) do { \
		GLenum err = glGetError(); \
		if (err != GL_NO_ERROR) { \
			do { \
				RN_CHECK_LOG_ERROR(fn, err); \
				err = glGetError(); \
			} while (err != GL_NO_ERROR); \
			exit(1); \
		} \
	} while(0)

	#define RN_VALIDATE_PARAM(fn, param) do { \
		GLenum err = glGetError(); \
		if (err != GL_NO_ERROR) { \
			do { \
				RN_CHECK_LOG_ERROR_PARAM(fn, err, param); \
				err = glGetError(); \
			} while (err != GL_NO_ERROR); \
			exit(1); \
		} \
	} while(0)

	#define RN_CHECK(fn) do { \
		RN_SAVE_CALL(fn); \
		fn; \
		RN_VALIDATE(fn); \
	} while (0)

	#define RN_CHECK_PARAM(fn, param) do { \
		RN_SAVE_CALL(fn); \
		fn; \
		RN_VALIDATE_PARAM(fn, param); \
	} while (0)

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