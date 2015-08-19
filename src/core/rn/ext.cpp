#include <pch.hpp>

#include "ext.hpp"

#include "../rn.hpp"

#include <iostream>

namespace rn
{

using namespace std;

Ext::Ext(string &&name)
	: name{move(name)}
{
	ext::list.push_back(this);
}

bool Ext::test() const
{
	if (name.empty())
	{
		throw string{"rn::Ext::test() - name not set"};
	}

	GLuint num;
	rn::get(GL_NUM_EXTENSIONS, num);

	for (GLuint i = 0; i < num; i++)
	{
		const char *ext = reinterpret_cast<const char *>(glGetStringi(GL_EXTENSIONS, i));

		if (name == string{ext})
		{
			return true;
		}
	}

	return false;
}

namespace ext
{

vector<Ext *> list{};

rn::Ext ARB_ES2_compatibility{"GL_ARB_ES2_compatibility"};
rn::Ext ARB_ES3_1_compatibility{"GL_ARB_ES3_1_compatibility"};
rn::Ext ARB_ES3_compatibility{"GL_ARB_ES3_compatibility"};
rn::Ext ARB_arrays_of_arrays{"GL_ARB_arrays_of_arrays"};
rn::Ext ARB_base_instance{"GL_ARB_base_instance"};
rn::Ext ARB_bindless_texture{"GL_ARB_bindless_texture"};
rn::Ext ARB_blend_func_extended{"GL_ARB_blend_func_extended"};
rn::Ext ARB_buffer_storage{"GL_ARB_buffer_storage"};
rn::Ext ARB_cl_event{"GL_ARB_cl_event"};
rn::Ext ARB_clear_buffer_object{"GL_ARB_clear_buffer_object"};
rn::Ext ARB_clear_texture{"GL_ARB_clear_texture"};
rn::Ext ARB_clip_control{"GL_ARB_clip_control"};
rn::Ext ARB_compressed_texture_pixel_storage{"GL_ARB_compressed_texture_pixel_storage"};
rn::Ext ARB_compute_shader{"GL_ARB_compute_shader"};
rn::Ext ARB_compute_variable_group_size{"GL_ARB_compute_variable_group_size"};
rn::Ext ARB_conditional_render_inverted{"GL_ARB_conditional_render_inverted"};
rn::Ext ARB_conservative_depth{"GL_ARB_conservative_depth"};
rn::Ext ARB_copy_buffer{"GL_ARB_copy_buffer"};
rn::Ext ARB_copy_image{"GL_ARB_copy_image"};
rn::Ext ARB_cull_distance{"GL_ARB_cull_distance"};
rn::Ext ARB_debug_output{"GL_ARB_debug_output"};
rn::Ext ARB_depth_buffer_float{"GL_ARB_depth_buffer_float"};
rn::Ext ARB_depth_clamp{"GL_ARB_depth_clamp"};
rn::Ext ARB_derivative_control{"GL_ARB_derivative_control"};
rn::Ext ARB_direct_state_access{"GL_ARB_direct_state_access"};
rn::Ext ARB_draw_buffers_blend{"GL_ARB_draw_buffers_blend"};
rn::Ext ARB_draw_elements_base_vertex{"GL_ARB_draw_elements_base_vertex"};
rn::Ext ARB_draw_indirect{"GL_ARB_draw_indirect"};
rn::Ext ARB_enhanced_layouts{"GL_ARB_enhanced_layouts"};
rn::Ext ARB_explicit_attrib_location{"GL_ARB_explicit_attrib_location"};
rn::Ext ARB_explicit_uniform_location{"GL_ARB_explicit_uniform_location"};
rn::Ext ARB_fragment_coord_conventions{"GL_ARB_fragment_coord_conventions"};
rn::Ext ARB_fragment_layer_viewport{"GL_ARB_fragment_layer_viewport"};
rn::Ext ARB_framebuffer_no_attachments{"GL_ARB_framebuffer_no_attachments"};
rn::Ext ARB_framebuffer_object{"GL_ARB_framebuffer_object"};
rn::Ext ARB_framebuffer_sRGB{"GL_ARB_framebuffer_sRGB"};
rn::Ext ARB_get_program_binary{"GL_ARB_get_program_binary"};
rn::Ext ARB_get_texture_sub_image{"GL_ARB_get_texture_sub_image"};
rn::Ext ARB_gpu_shader5{"GL_ARB_gpu_shader5"};
rn::Ext ARB_gpu_shader_fp64{"GL_ARB_gpu_shader_fp64"};
rn::Ext ARB_half_float_vertex{"GL_ARB_half_float_vertex"};
rn::Ext ARB_imaging{"GL_ARB_imaging"};
rn::Ext ARB_indirect_parameters{"GL_ARB_indirect_parameters"};
rn::Ext ARB_internalformat_query{"GL_ARB_internalformat_query"};
rn::Ext ARB_internalformat_query2{"GL_ARB_internalformat_query2"};
rn::Ext ARB_invalidate_subdata{"GL_ARB_invalidate_subdata"};
rn::Ext ARB_map_buffer_alignment{"GL_ARB_map_buffer_alignment"};
rn::Ext ARB_map_buffer_range{"GL_ARB_map_buffer_range"};
rn::Ext ARB_multi_bind{"GL_ARB_multi_bind"};
rn::Ext ARB_multi_draw_indirect{"GL_ARB_multi_draw_indirect"};
rn::Ext ARB_occlusion_query2{"GL_ARB_occlusion_query2"};
rn::Ext ARB_pipeline_statistics_query{"GL_ARB_pipeline_statistics_query"};
rn::Ext ARB_program_interface_query{"GL_ARB_program_interface_query"};
rn::Ext ARB_provoking_vertex{"GL_ARB_provoking_vertex"};
rn::Ext ARB_query_buffer_object{"GL_ARB_query_buffer_object"};
rn::Ext ARB_robust_buffer_access_behavior{"GL_ARB_robust_buffer_access_behavior"};
rn::Ext ARB_robustness{"GL_ARB_robustness"};
rn::Ext ARB_robustness_isolation{"GL_ARB_robustness_isolation"};
rn::Ext ARB_sample_shading{"GL_ARB_sample_shading"};
rn::Ext ARB_sampler_objects{"GL_ARB_sampler_objects"};
rn::Ext ARB_seamless_cube_map{"GL_ARB_seamless_cube_map"};
rn::Ext ARB_seamless_cubemap_per_texture{"GL_ARB_seamless_cubemap_per_texture"};
rn::Ext ARB_separate_shader_objects{"GL_ARB_separate_shader_objects"};
rn::Ext ARB_shader_atomic_counters{"GL_ARB_shader_atomic_counters"};
rn::Ext ARB_shader_bit_encoding{"GL_ARB_shader_bit_encoding"};
rn::Ext ARB_shader_draw_parameters{"GL_ARB_shader_draw_parameters"};
rn::Ext ARB_shader_group_vote{"GL_ARB_shader_group_vote"};
rn::Ext ARB_shader_image_load_store{"GL_ARB_shader_image_load_store"};
rn::Ext ARB_shader_image_size{"GL_ARB_shader_image_size"};
rn::Ext ARB_shader_precision{"GL_ARB_shader_precision"};
rn::Ext ARB_shader_stencil_export{"GL_ARB_shader_stencil_export"};
rn::Ext ARB_shader_storage_buffer_object{"GL_ARB_shader_storage_buffer_object"};
rn::Ext ARB_shader_subroutine{"GL_ARB_shader_subroutine"};
rn::Ext ARB_shader_texture_image_samples{"GL_ARB_shader_texture_image_samples"};
rn::Ext ARB_shading_language_420pack{"GL_ARB_shading_language_420pack"};
rn::Ext ARB_shading_language_include{"GL_ARB_shading_language_include"};
rn::Ext ARB_shading_language_packing{"GL_ARB_shading_language_packing"};
rn::Ext ARB_sparse_buffer{"GL_ARB_sparse_buffer"};
rn::Ext ARB_sparse_texture{"GL_ARB_sparse_texture"};
rn::Ext ARB_stencil_texturing{"GL_ARB_stencil_texturing"};
rn::Ext ARB_sync{"GL_ARB_sync"};
rn::Ext ARB_tessellation_shader{"GL_ARB_tessellation_shader"};
rn::Ext ARB_texture_barrier{"GL_ARB_texture_barrier"};
rn::Ext ARB_texture_buffer_object_rgb32{"GL_ARB_texture_buffer_object_rgb32"};
rn::Ext ARB_texture_buffer_range{"GL_ARB_texture_buffer_range"};
rn::Ext ARB_texture_compression_bptc{"GL_ARB_texture_compression_bptc"};
rn::Ext ARB_texture_compression_rgtc{"GL_ARB_texture_compression_rgtc"};
rn::Ext ARB_texture_cube_map_array{"GL_ARB_texture_cube_map_array"};
rn::Ext ARB_texture_gather{"GL_ARB_texture_gather"};
rn::Ext ARB_texture_mirror_clamp_to_edge{"GL_ARB_texture_mirror_clamp_to_edge"};
rn::Ext ARB_texture_multisample{"GL_ARB_texture_multisample"};
rn::Ext ARB_texture_query_levels{"GL_ARB_texture_query_levels"};
rn::Ext ARB_texture_query_lod{"GL_ARB_texture_query_lod"};
rn::Ext ARB_texture_rg{"GL_ARB_texture_rg"};
rn::Ext ARB_texture_rgb10_a2ui{"GL_ARB_texture_rgb10_a2ui"};
rn::Ext ARB_texture_stencil8{"GL_ARB_texture_stencil8"};
rn::Ext ARB_texture_storage{"GL_ARB_texture_storage"};
rn::Ext ARB_texture_storage_multisample{"GL_ARB_texture_storage_multisample"};
rn::Ext ARB_texture_swizzle{"GL_ARB_texture_swizzle"};
rn::Ext ARB_texture_view{"GL_ARB_texture_view"};
rn::Ext ARB_timer_query{"GL_ARB_timer_query"};
rn::Ext ARB_transform_feedback2{"GL_ARB_transform_feedback2"};
rn::Ext ARB_transform_feedback3{"GL_ARB_transform_feedback3"};
rn::Ext ARB_transform_feedback_instanced{"GL_ARB_transform_feedback_instanced"};
rn::Ext ARB_transform_feedback_overflow_query{"GL_ARB_transform_feedback_overflow_query"};
rn::Ext ARB_uniform_buffer_object{"GL_ARB_uniform_buffer_object"};
rn::Ext ARB_vertex_array_bgra{"GL_ARB_vertex_array_bgra"};
rn::Ext ARB_vertex_array_object{"GL_ARB_vertex_array_object"};
rn::Ext ARB_vertex_attrib_64bit{"GL_ARB_vertex_attrib_64bit"};
rn::Ext ARB_vertex_attrib_binding{"GL_ARB_vertex_attrib_binding"};
rn::Ext ARB_vertex_type_10f_11f_11f_rev{"GL_ARB_vertex_type_10f_11f_11f_rev"};
rn::Ext ARB_vertex_type_2_10_10_10_rev{"GL_ARB_vertex_type_2_10_10_10_rev"};
rn::Ext ARB_viewport_array{"GL_ARB_viewport_array"};
rn::Ext KHR_context_flush_control{"GL_KHR_context_flush_control"};
rn::Ext KHR_debug{"GL_KHR_debug"};
rn::Ext KHR_no_error{"GL_KHR_no_error"};
rn::Ext KHR_robust_buffer_access_behavior{"GL_KHR_robust_buffer_access_behavior"};
rn::Ext KHR_robustness{"GL_KHR_robustness"};
rn::Ext KHR_texture_compression_astc_hdr{"GL_KHR_texture_compression_astc_hdr"};
rn::Ext KHR_texture_compression_astc_ldr{"GL_KHR_texture_compression_astc_ldr"};

} // ext

} // rn