#pragma once

#include <vector>
#include <string>

namespace rn
{

class Ext
{
public:

	const std::string name;

	explicit Ext(std::string &&name);

	int result = -1;

	bool test() const;

	operator bool()
	{
		return asBool();
	}

	bool asBool()
	{
		if (result == -1)
		{
			result = test() ? 1 : 0;
		}

		return result > 0;
	}
};

namespace ext
{

extern std::vector<Ext *> list;

extern rn::Ext ARB_ES2_compatibility;
extern rn::Ext ARB_ES3_1_compatibility;
extern rn::Ext ARB_ES3_compatibility;
extern rn::Ext ARB_arrays_of_arrays;
extern rn::Ext ARB_base_instance;
extern rn::Ext ARB_bindless_texture;
extern rn::Ext ARB_blend_func_extended;
extern rn::Ext ARB_buffer_storage;
extern rn::Ext ARB_cl_event;
extern rn::Ext ARB_clear_buffer_object;
extern rn::Ext ARB_clear_texture;
extern rn::Ext ARB_clip_control;
extern rn::Ext ARB_compressed_texture_pixel_storage;
extern rn::Ext ARB_compute_shader;
extern rn::Ext ARB_compute_variable_group_size;
extern rn::Ext ARB_conditional_render_inverted;
extern rn::Ext ARB_conservative_depth;
extern rn::Ext ARB_copy_buffer;
extern rn::Ext ARB_copy_image;
extern rn::Ext ARB_cull_distance;
extern rn::Ext ARB_debug_output;
extern rn::Ext ARB_depth_buffer_float;
extern rn::Ext ARB_depth_clamp;
extern rn::Ext ARB_derivative_control;
extern rn::Ext ARB_direct_state_access;
extern rn::Ext ARB_draw_buffers_blend;
extern rn::Ext ARB_draw_elements_base_vertex;
extern rn::Ext ARB_draw_indirect;
extern rn::Ext ARB_enhanced_layouts;
extern rn::Ext ARB_explicit_attrib_location;
extern rn::Ext ARB_explicit_uniform_location;
extern rn::Ext ARB_fragment_coord_conventions;
extern rn::Ext ARB_fragment_layer_viewport;
extern rn::Ext ARB_framebuffer_no_attachments;
extern rn::Ext ARB_framebuffer_object;
extern rn::Ext ARB_framebuffer_sRGB;
extern rn::Ext ARB_get_program_binary;
extern rn::Ext ARB_get_texture_sub_image;
extern rn::Ext ARB_gpu_shader5;
extern rn::Ext ARB_gpu_shader_fp64;
extern rn::Ext ARB_half_float_vertex;
extern rn::Ext ARB_imaging;
extern rn::Ext ARB_indirect_parameters;
extern rn::Ext ARB_internalformat_query;
extern rn::Ext ARB_internalformat_query2;
extern rn::Ext ARB_invalidate_subdata;
extern rn::Ext ARB_map_buffer_alignment;
extern rn::Ext ARB_map_buffer_range;
extern rn::Ext ARB_multi_bind;
extern rn::Ext ARB_multi_draw_indirect;
extern rn::Ext ARB_occlusion_query2;
extern rn::Ext ARB_pipeline_statistics_query;
extern rn::Ext ARB_program_interface_query;
extern rn::Ext ARB_provoking_vertex;
extern rn::Ext ARB_query_buffer_object;
extern rn::Ext ARB_robust_buffer_access_behavior;
extern rn::Ext ARB_robustness;
extern rn::Ext ARB_robustness_isolation;
extern rn::Ext ARB_sample_shading;
extern rn::Ext ARB_sampler_objects;
extern rn::Ext ARB_seamless_cube_map;
extern rn::Ext ARB_seamless_cubemap_per_texture;
extern rn::Ext ARB_separate_shader_objects;
extern rn::Ext ARB_shader_atomic_counters;
extern rn::Ext ARB_shader_bit_encoding;
extern rn::Ext ARB_shader_draw_parameters;
extern rn::Ext ARB_shader_group_vote;
extern rn::Ext ARB_shader_image_load_store;
extern rn::Ext ARB_shader_image_size;
extern rn::Ext ARB_shader_precision;
extern rn::Ext ARB_shader_stencil_export;
extern rn::Ext ARB_shader_storage_buffer_object;
extern rn::Ext ARB_shader_subroutine;
extern rn::Ext ARB_shader_texture_image_samples;
extern rn::Ext ARB_shading_language_420pack;
extern rn::Ext ARB_shading_language_include;
extern rn::Ext ARB_shading_language_packing;
extern rn::Ext ARB_sparse_buffer;
extern rn::Ext ARB_sparse_texture;
extern rn::Ext ARB_stencil_texturing;
extern rn::Ext ARB_sync;
extern rn::Ext ARB_tessellation_shader;
extern rn::Ext ARB_texture_barrier;
extern rn::Ext ARB_texture_buffer_object_rgb32;
extern rn::Ext ARB_texture_buffer_range;
extern rn::Ext ARB_texture_compression_bptc;
extern rn::Ext ARB_texture_compression_rgtc;
extern rn::Ext ARB_texture_cube_map_array;
extern rn::Ext ARB_texture_gather;
extern rn::Ext ARB_texture_mirror_clamp_to_edge;
extern rn::Ext ARB_texture_multisample;
extern rn::Ext ARB_texture_query_levels;
extern rn::Ext ARB_texture_query_lod;
extern rn::Ext ARB_texture_rg;
extern rn::Ext ARB_texture_rgb10_a2ui;
extern rn::Ext ARB_texture_stencil8;
extern rn::Ext ARB_texture_storage;
extern rn::Ext ARB_texture_storage_multisample;
extern rn::Ext ARB_texture_swizzle;
extern rn::Ext ARB_texture_view;
extern rn::Ext ARB_timer_query;
extern rn::Ext ARB_transform_feedback2;
extern rn::Ext ARB_transform_feedback3;
extern rn::Ext ARB_transform_feedback_instanced;
extern rn::Ext ARB_transform_feedback_overflow_query;
extern rn::Ext ARB_uniform_buffer_object;
extern rn::Ext ARB_vertex_array_bgra;
extern rn::Ext ARB_vertex_array_object;
extern rn::Ext ARB_vertex_attrib_64bit;
extern rn::Ext ARB_vertex_attrib_binding;
extern rn::Ext ARB_vertex_type_10f_11f_11f_rev;
extern rn::Ext ARB_vertex_type_2_10_10_10_rev;
extern rn::Ext ARB_viewport_array;
extern rn::Ext KHR_context_flush_control;
extern rn::Ext KHR_debug;
extern rn::Ext KHR_no_error;
extern rn::Ext KHR_robust_buffer_access_behavior;
extern rn::Ext KHR_robustness;
extern rn::Ext KHR_texture_compression_astc_hdr;
extern rn::Ext KHR_texture_compression_astc_ldr;

} // ext

} // rn