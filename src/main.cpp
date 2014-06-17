#include <array>
#include <sstream>
#include <iostream>
#include <iomanip>

// #include "core/cam/basic.hpp"
#include "core/gl.hpp"
// #include "core/gl/fbo.hpp"
#include "core/gl/font.hpp"
// #include "core/gl/mesh.hpp"
// #include "core/gl/program.hpp"
// #include "core/util.hpp"
#include "core/util/align.hpp"
#include "core/util/count.hpp"
#include "core/util/scope.hpp"
// #include "core/util/initq.hpp"
#include "core/ngn.hpp"
#include "core/ngn/fs.hpp"
#include "core/ngn/key.hpp"
#include "core/ngn/loop.hpp"
#include "core/ngn/window.hpp"
// #include "core/src/mem.hpp"
// #include "core/src/sbm.hpp"

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
			gl::getBasicInfo();
			clog << "OpenGL info:" << endl;
			clog << gl::getBasicInfo("  ") << endl;

			if (argc > 1 && argv[1] == string{"--print-exts"})
			{
				clog << "  Extensions:" << endl;
				clog << gl::getExtensionsInfo("    ") << endl;
			}
		}

		App app{};
		app.init();

		gl::Font font{"DejaVuSansMono"};
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
			gl::reloadSoftAll();
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
					gl::Program::reloadAll();
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
				gl::Mesh::reloadAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F7))
			{
				cout << "Reloading fonts..." << endl;
				gl::Font::reloadAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F8))
			{
				cout << "Reloading FBOs..." << endl;
				gl::FBO::reloadAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F10))
			{
				currentVsync = (currentVsync + 1) % util::countOf(vsyncs);

				cout << "Switching vsync mode to \"" << vsyncNames[currentVsync] << "\" (" << vsyncs[currentVsync] << ") ..." << endl;
				win::switchMode(modes[currentMode]);
				win::vsync(vsyncs[currentVsync]);
				gl::reloadSoftAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F11))
			{
				cout << "Switching window mode..." << endl;
				currentMode = (currentMode + 1) % util::countOf(modes);
				win::switchMode(modes[currentMode]);
				win::vsync(vsyncs[currentVsync]);
				cout << "done" << endl;

				cout << "Soft-reloading GL..." << endl;
				gl::reloadSoftAll();
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
					oss << "triangles=" << gl::stats.triangles << "\n";
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