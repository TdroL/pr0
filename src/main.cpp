#include <array>
#include <sstream>
#include <iostream>
#include <iomanip>

// #include <core/cam/basic.hpp>
#include <core/rn.hpp>
// #include <core/rn/fbo.hpp>
#include <core/rn/font.hpp>
// #include <core/rn/mesh.hpp>
// #include <core/rn/program.hpp>
// #include <core/util.hpp>
#include <core/util/align.hpp>
#include <core/util/count.hpp>
#include <core/util/scope.hpp>
// #include <core/util/initq.hpp>
#include <core/ngn.hpp>
#include <core/ngn/fs.hpp>
#include <core/ngn/key.hpp>
#include <core/ngn/loop.hpp>
#include <core/ngn/window.hpp>
// #include <core/src/mem.hpp>
// #include <core/src/sbm.hpp>

#include "app.hpp"

using namespace std;

namespace fs = ngn::fs;
namespace key = ngn::key;
namespace win = ngn::window;

int main(int argc, char const* argv[])
{
	try
	{
		ngn::init();

		UTIL_SCOPE_EXIT([] () {
			ngn::deinit();
		});

		UTIL_DEBUG
		{
			for (int i = 1; i < argc; ++i)
			{
				if (argv[i] == string{"--print-exts"})
				{
					clog << "Extensions:" << endl;
					clog << rn::getExtensionsInfo() << endl;
				}
			}
		}

		cout << "GL_ARB_arrays_of_arrays = " << (bool) rn::ext_ARB_arrays_of_arrays << endl;
		cout << "GL_ARB_base_instance = " << (bool) rn::ext_ARB_base_instance << endl;
		cout << "GL_ARB_blend_func_extended = " << (bool) rn::ext_ARB_blend_func_extended << endl;
		cout << "GL_ARB_cl_event = " << (bool) rn::ext_ARB_cl_event << endl;
		cout << "GL_ARB_clear_buffer_object = " << (bool) rn::ext_ARB_clear_buffer_object << endl;
		cout << "GL_ARB_compressed_texture_pixel_storage = " << (bool) rn::ext_ARB_compressed_texture_pixel_storage << endl;
		cout << "GL_ARB_compute_shader = " << (bool) rn::ext_ARB_compute_shader << endl;
		cout << "GL_ARB_conservative_depth = " << (bool) rn::ext_ARB_conservative_depth << endl;
		cout << "GL_ARB_copy_buffer = " << (bool) rn::ext_ARB_copy_buffer << endl;
		cout << "GL_ARB_copy_image = " << (bool) rn::ext_ARB_copy_image << endl;
		cout << "GL_ARB_debug_output = " << (bool) rn::ext_ARB_debug_output << endl;
		cout << "GL_ARB_depth_buffer_float = " << (bool) rn::ext_ARB_depth_buffer_float << endl;
		cout << "GL_ARB_depth_clamp = " << (bool) rn::ext_ARB_depth_clamp << endl;
		cout << "GL_ARB_draw_buffers_blend = " << (bool) rn::ext_ARB_draw_buffers_blend << endl;
		cout << "GL_ARB_draw_elements_base_vertex = " << (bool) rn::ext_ARB_draw_elements_base_vertex << endl;
		cout << "GL_ARB_draw_indirect = " << (bool) rn::ext_ARB_draw_indirect << endl;
		cout << "GL_ARB_ES2_compatibility = " << (bool) rn::ext_ARB_ES2_compatibility << endl;
		cout << "GL_ARB_ES3_compatibility = " << (bool) rn::ext_ARB_ES3_compatibility << endl;
		cout << "GL_ARB_explicit_attrib_location = " << (bool) rn::ext_ARB_explicit_attrib_location << endl;
		cout << "GL_ARB_explicit_uniform_location = " << (bool) rn::ext_ARB_explicit_uniform_location << endl;
		cout << "GL_ARB_fragment_coord_conventions = " << (bool) rn::ext_ARB_fragment_coord_conventions << endl;
		cout << "GL_ARB_fragment_layer_viewport = " << (bool) rn::ext_ARB_fragment_layer_viewport << endl;
		cout << "GL_ARB_framebuffer_no_attachments = " << (bool) rn::ext_ARB_framebuffer_no_attachments << endl;
		cout << "GL_ARB_framebuffer_object = " << (bool) rn::ext_ARB_framebuffer_object << endl;
		cout << "GL_ARB_framebuffer_sRGB = " << (bool) rn::ext_ARB_framebuffer_sRGB << endl;
		cout << "GL_ARB_get_program_binary = " << (bool) rn::ext_ARB_get_program_binary << endl;
		cout << "GL_ARB_gpu_shader5 = " << (bool) rn::ext_ARB_gpu_shader5 << endl;
		cout << "GL_ARB_gpu_shader_fp64 = " << (bool) rn::ext_ARB_gpu_shader_fp64 << endl;
		cout << "GL_ARB_half_float_pixel = " << (bool) rn::ext_ARB_half_float_pixel << endl;
		cout << "GL_ARB_half_float_vertex = " << (bool) rn::ext_ARB_half_float_vertex << endl;
		cout << "GL_ARB_imaging = " << (bool) rn::ext_ARB_imaging << endl;
		cout << "GL_ARB_internalformat_query = " << (bool) rn::ext_ARB_internalformat_query << endl;
		cout << "GL_ARB_internalformat_query2 = " << (bool) rn::ext_ARB_internalformat_query2 << endl;
		cout << "GL_ARB_invalidate_subdata = " << (bool) rn::ext_ARB_invalidate_subdata << endl;
		cout << "GL_ARB_map_buffer_alignment = " << (bool) rn::ext_ARB_map_buffer_alignment << endl;
		cout << "GL_ARB_map_buffer_range = " << (bool) rn::ext_ARB_map_buffer_range << endl;
		cout << "GL_ARB_multi_draw_indirect = " << (bool) rn::ext_ARB_multi_draw_indirect << endl;
		cout << "GL_ARB_occlusion_query2 = " << (bool) rn::ext_ARB_occlusion_query2 << endl;
		cout << "GL_ARB_program_interface_query = " << (bool) rn::ext_ARB_program_interface_query << endl;
		cout << "GL_ARB_provoking_vertex = " << (bool) rn::ext_ARB_provoking_vertex << endl;
		cout << "GL_ARB_robust_buffer_access_behavior = " << (bool) rn::ext_ARB_robust_buffer_access_behavior << endl;
		cout << "GL_ARB_robustness = " << (bool) rn::ext_ARB_robustness << endl;
		cout << "GL_ARB_robustness_isolation = " << (bool) rn::ext_ARB_robustness_isolation << endl;
		cout << "GL_ARB_sample_shading = " << (bool) rn::ext_ARB_sample_shading << endl;
		cout << "GL_ARB_sampler_objects = " << (bool) rn::ext_ARB_sampler_objects << endl;
		cout << "GL_ARB_seamless_cube_map = " << (bool) rn::ext_ARB_seamless_cube_map << endl;
		cout << "GL_ARB_separate_shader_objects = " << (bool) rn::ext_ARB_separate_shader_objects << endl;
		cout << "GL_ARB_shader_atomic_counters = " << (bool) rn::ext_ARB_shader_atomic_counters << endl;
		cout << "GL_ARB_shader_bit_encoding = " << (bool) rn::ext_ARB_shader_bit_encoding << endl;
		cout << "GL_ARB_shader_image_load_store = " << (bool) rn::ext_ARB_shader_image_load_store << endl;
		cout << "GL_ARB_shader_image_size = " << (bool) rn::ext_ARB_shader_image_size << endl;
		cout << "GL_ARB_shader_objects = " << (bool) rn::ext_ARB_shader_objects << endl;
		cout << "GL_ARB_shader_precision = " << (bool) rn::ext_ARB_shader_precision << endl;
		cout << "GL_ARB_shader_stencil_export = " << (bool) rn::ext_ARB_shader_stencil_export << endl;
		cout << "GL_ARB_shader_storage_buffer_object = " << (bool) rn::ext_ARB_shader_storage_buffer_object << endl;
		cout << "GL_ARB_shader_subroutine = " << (bool) rn::ext_ARB_shader_subroutine << endl;
		cout << "GL_ARB_shading_language_420pack = " << (bool) rn::ext_ARB_shading_language_420pack << endl;
		cout << "GL_ARB_shading_language_include = " << (bool) rn::ext_ARB_shading_language_include << endl;
		cout << "GL_ARB_shading_language_packing = " << (bool) rn::ext_ARB_shading_language_packing << endl;
		cout << "GL_ARB_stencil_texturing = " << (bool) rn::ext_ARB_stencil_texturing << endl;
		cout << "GL_ARB_sync = " << (bool) rn::ext_ARB_sync << endl;
		cout << "GL_ARB_tessellation_shader = " << (bool) rn::ext_ARB_tessellation_shader << endl;
		cout << "GL_ARB_texture_buffer_object_rgb32 = " << (bool) rn::ext_ARB_texture_buffer_object_rgb32 << endl;
		cout << "GL_ARB_texture_buffer_range = " << (bool) rn::ext_ARB_texture_buffer_range << endl;
		cout << "GL_ARB_texture_compression_bptc = " << (bool) rn::ext_ARB_texture_compression_bptc << endl;
		cout << "GL_ARB_texture_compression_rgtc = " << (bool) rn::ext_ARB_texture_compression_rgtc << endl;
		cout << "GL_ARB_texture_cube_map_array = " << (bool) rn::ext_ARB_texture_cube_map_array << endl;
		cout << "GL_ARB_texture_gather = " << (bool) rn::ext_ARB_texture_gather << endl;
		cout << "GL_ARB_texture_multisample = " << (bool) rn::ext_ARB_texture_multisample << endl;
		cout << "GL_ARB_texture_query_levels = " << (bool) rn::ext_ARB_texture_query_levels << endl;
		cout << "GL_ARB_texture_query_lod = " << (bool) rn::ext_ARB_texture_query_lod << endl;
		cout << "GL_ARB_texture_rg = " << (bool) rn::ext_ARB_texture_rg << endl;
		cout << "GL_ARB_texture_rgb10_a2ui = " << (bool) rn::ext_ARB_texture_rgb10_a2ui << endl;
		cout << "GL_ARB_texture_storage = " << (bool) rn::ext_ARB_texture_storage << endl;
		cout << "GL_ARB_texture_storage_multisample = " << (bool) rn::ext_ARB_texture_storage_multisample << endl;
		cout << "GL_ARB_texture_swizzle = " << (bool) rn::ext_ARB_texture_swizzle << endl;
		cout << "GL_ARB_texture_view = " << (bool) rn::ext_ARB_texture_view << endl;
		cout << "GL_ARB_timer_query = " << (bool) rn::ext_ARB_timer_query << endl;
		cout << "GL_ARB_transform_feedback2 = " << (bool) rn::ext_ARB_transform_feedback2 << endl;
		cout << "GL_ARB_transform_feedback3 = " << (bool) rn::ext_ARB_transform_feedback3 << endl;
		cout << "GL_ARB_transform_feedback_instanced = " << (bool) rn::ext_ARB_transform_feedback_instanced << endl;
		cout << "GL_ARB_uniform_buffer_object = " << (bool) rn::ext_ARB_uniform_buffer_object << endl;
		cout << "GL_ARB_vertex_array_bgra = " << (bool) rn::ext_ARB_vertex_array_bgra << endl;
		cout << "GL_ARB_vertex_array_object = " << (bool) rn::ext_ARB_vertex_array_object << endl;
		cout << "GL_ARB_vertex_attrib_64bit = " << (bool) rn::ext_ARB_vertex_attrib_64bit << endl;
		cout << "GL_ARB_vertex_attrib_binding = " << (bool) rn::ext_ARB_vertex_attrib_binding << endl;
		cout << "GL_ARB_vertex_buffer_object = " << (bool) rn::ext_ARB_vertex_buffer_object << endl;
		cout << "GL_ARB_vertex_type_2_10_10_10_rev = " << (bool) rn::ext_ARB_vertex_type_2_10_10_10_rev << endl;
		cout << "GL_ARB_viewport_array = " << (bool) rn::ext_ARB_viewport_array << endl;
		cout << "GL_KHR_debug = " << (bool) rn::ext_KHR_debug << endl;
		cout << "GL_KHR_texture_compression_astc_ldr = " << (bool) rn::ext_KHR_texture_compression_astc_ldr << endl;

		App app{};
		app.init();

		rn::Font font{"DejaVuSansMono"};
		font.load("DejaVu/DejaVuSansMono.ttf");

		const win::Mode modes[] {
			win::Mode::windowed,
			win::Mode::borderless,
			win::Mode::fullscreen
		};
		size_t currentMode = 0;

		const int vsyncs[] {
			-1, // progressive
			 0, // off
			 1, // on
		};
		const string vsyncNames[] {
			"progressive",
			"off",
			"on",
		};
		size_t currentVsync = 0;

		/* Test: switch to window mode */
		UTIL_DEBUG
		{
			currentMode = 0;
			currentVsync = 0;

			win::switchMode(modes[currentMode]);
			win::vsync(vsyncs[currentVsync]);
			rn::reloadSoftAll();
		}

		/* Refresh system */

		ngn::update();

		while ( ! win::shouldClose())
		{
			NGN_LOOP;

			if (key::hit(KEY_ESC))
			{
				win::close();
			}

			if (key::hit(KEY_F5))
			{
				cout << "Reloading shaders..." << endl;
				try
				{
					rn::Program::reloadAll();
					cout << "done" << endl;
				}
				catch (string e)
				{
					cerr << "  - " << e << endl;
				}
			}

			if (key::hit(KEY_F6))
			{
				cout << "Reloading meshes..." << endl;
				rn::Mesh::reloadAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F7))
			{
				cout << "Reloading fonts..." << endl;
				rn::Font::reloadAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F8))
			{
				cout << "Reloading FBOs..." << endl;
				rn::FBO::reloadAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F10))
			{
				currentVsync = (currentVsync + 1) % util::countOf(vsyncs);

				cout << "Switching vsync mode to \"" << vsyncNames[currentVsync] << "\" (" << vsyncs[currentVsync] << ") ..." << endl;
				win::switchMode(modes[currentMode]);
				win::vsync(vsyncs[currentVsync]);
				rn::reloadSoftAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F11))
			{
				cout << "Switching window mode..." << endl;
				currentMode = (currentMode + 1) % util::countOf(modes);
				win::switchMode(modes[currentMode]);
				win::vsync(vsyncs[currentVsync]);
				cout << "done" << endl;

				cout << "Soft reloading GL..." << endl;
				rn::reloadSoftAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_ESC))
			{
				win::close();
				continue;
			}

			app.update();

			app.render();

			{
				double ft = ngn::time() - ngn::ct;

				{
					ostringstream oss;
					oss << setprecision(4) << fixed;
					oss << "dt=" << ngn::dt * 1000.0 << " ms\n";
					oss << "ft=" << ft * 1000.0 << " ms\n";
					oss << "fps=" << 1.0/ngn::dt << "\n";
					oss << "fps=" << 1.0/ft << " (frame)\n";
					oss << "triangles=" << rn::stats.triangles << "\n";
					oss << "\n";
					oss << "F5 - reload shaders\n";
					oss << "F6 - reload meshes\n";
					oss << "F7 - reload fonts\n";
					oss << "F8 - reload FBOs\n";
					oss << "\n";
					oss << "F10 - change vsync mode (current: " << vsyncNames[currentVsync] << ")\n";
					oss << "F11 - change window mode\n";
					oss << "\n";
					oss << "Movement: W, A, S, D\n";
					oss << "Camera: arrows\n";
					oss << "Point Light: Keypad 8, 4, 5, 6\n";
					font.render(oss.str());
				}
			}
		}
	}
	catch (const exception &e)
	{
		cerr << "Exception: " << e.what() << endl;
		return EXIT_FAILURE;
	}
	catch (const string &e)
	{
		cerr << "Exception: " << e << endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		cerr << "Unknown exception" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}