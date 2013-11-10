#include <array>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "core/gl.hpp"
#include "core/gl/font.hpp"
#include "core/gl/mesh.hpp"
#include "core/gl/program.hpp"
#include "core/util.hpp"
#include "core/sys.hpp"
#include "core/sys/fs.hpp"
#include "core/sys/key.hpp"
#include "core/sys/loop.hpp"
#include "core/sys/window.hpp"
#include "core/src/mem.hpp"
#include "core/sg/root.hpp"
#include "core/sm/machine.hpp"
#include "core/sm/state.hpp"

using namespace std;

namespace fs = sys::fs;

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

		sys::window::vsync(0);

		gl::Program prog{};
		gl::Mesh dummy{};
		gl::Font font{};

		prog.load("color.frag", "P.vert");

		prog.uniform("color", 1.0, 1.0, 1.0);

		auto &&mesh = src::mem::mesh({
			 1.0f,  1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
		});

		mesh->arrays.emplace_back(GL_TRIANGLE_STRIP, 0, 4);
		mesh->layouts.emplace_back(0, 3, GL_FLOAT, 0, 0);

		dummy.load(move(mesh));

		font.load("DejaVu/DejaVuSansMono.ttf");

		sys::update();

		const sys::window::Mode modes[] {
			sys::window::Mode::windowed,
			sys::window::Mode::borderless,
			sys::window::Mode::fullscreen
		};
		int currentMode = 0;

		while ( ! sys::window::shouldClose())
		{
			SYS_LOOP;

			if (sys::key::hit(KEY_ESC))
			{
				sys::window::close();
			}

			if (sys::key::hit(KEY_F5))
			{
				cout << "Reloading shaders... ";
				gl::Program::reloadAll();
				cout << "done" << endl;
			}

			if (sys::key::hit(KEY_F6))
			{
				cout << "Reloading meshes... ";
				gl::Mesh::reloadAll();
				cout << "done" << endl;
			}

			if (sys::key::hit(KEY_F7))
			{
				cout << "Reloading fonts... ";
				gl::Font::reloadAll();
				cout << "done" << endl;
			}

			if (sys::key::hit(KEY_F11))
			{
				cout << "Switching window mode... ";
				currentMode = (currentMode + 1) % util::countOf(modes);
				sys::window::switchMode(modes[currentMode]);
				cout << "done" << endl;

				gl::reload();

				cout << "Reloading meshes... ";
				gl::Mesh::reloadAll();
				cout << "done" << endl;

				cout << "Reloading fonts... ";
				gl::Font::reloadAll();
				cout << "done" << endl;
			}

			if (sys::key::hit(KEY_ESC))
			{
				sys::window::close();
			}

			glClearColor(0.0f, 0.3125f, 1.0f, 1.0f);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			prog.use();

			// dummy.render();

			{
				double ft = sys::time() - sys::ct;

				ostringstream oss;
				oss << setprecision(4) << fixed;
				oss << "dt=" << sys::dt * 1000.0 << " ms\n";
				oss << "ft=" << ft * 1000.0 << " ms\n";
				oss << "fps=" << 1.0/sys::dt << "\n";
				oss << "fps=" << 1.0/ft << " (real)\n";

				font.render(oss.str());
			}

			// break;
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