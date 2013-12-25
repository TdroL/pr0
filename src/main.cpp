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
#include "core/util/count.hpp"
#include "core/util/scope.hpp"
// #include "core/util/initq.hpp"
#include "core/sys.hpp"
#include "core/sys/fs.hpp"
#include "core/sys/key.hpp"
#include "core/sys/loop.hpp"
#include "core/sys/window.hpp"
// #include "core/src/mem.hpp"
// #include "core/src/sbm.hpp"

#include "app.hpp"

using namespace std;

namespace fs = sys::fs;
namespace key = sys::key;
namespace win = sys::window;

int main(int argc, char const* argv[])
{
	try
	{
		sys::init();

		UTIL_SCOPE_EXIT([] () {
			sys::deinit();
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

		/* Test: switch to window mode */
		UTIL_DEBUG
		{
			win::switchMode(win::Mode::windowed);
			gl::reloadSoftAll();
		}

		/* Refresh system */

		sys::update();

		const win::Mode modes[] {
			win::Mode::windowed,
			win::Mode::borderless,
			win::Mode::fullscreen
		};
		int currentMode = 0;

		while ( ! win::shouldClose())
		{
			SYS_LOOP;

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

			if (key::hit(KEY_F11))
			{
				cout << "Switching window mode..." << endl;
				currentMode = (currentMode + 1) % util::countOf(modes);
				win::switchMode(modes[currentMode]);
				cout << "done" << endl;

				cout << "Soft-reloading GL..." << endl;
				gl::reloadAll();
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
				double ft = sys::time() - sys::ct;

				{
					ostringstream oss;
					oss << setprecision(4) << fixed;
					oss << "dt=" << sys::dt * 1000.0 << " ms\n";
					oss << "ft=" << ft * 1000.0 << " ms\n";
					oss << "fps=" << 1.0/sys::dt << "\n";
					oss << "fps=" << 1.0/ft << " (real)\n";
					oss << "\n"; // triangles=
					oss << "\n";
					oss << "\n";
					oss << "F5 - reload shaders\n";
					oss << "F6 - reload meshes\n";
					oss << "F7 - reload fonts\n";
					oss << "F8 - reload FBOs\n";
					oss << "\n";
					oss << "F11 - change window mode\n";
					font.render(oss.str());
				}

				{
					ostringstream oss;
					oss << "\n";
					oss << "\n";
					oss << "\n";
					oss << "\n";
					oss << "triangles=" << gl::stats.triangles << "\n";
					font.render(oss.str());
				}
			}
		}

		/**/
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