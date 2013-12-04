#include <array>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "core/cam/basic.hpp"
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
#include "core/src/obj.hpp"
#include "core/src/sbm.hpp"
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

		GLboolean cullFace;
		sys::window::switchMode(sys::window::Mode::windowed);
		gl::init();

		gl::Program prog{};
		gl::Mesh dummy{};
		gl::Mesh suzanne{};
		gl::Mesh venus{};
		gl::Font font{};

		cam::Basic camera{glm::vec3{0.f, 0.f, 5.f}};

		glm::mat4 projectionMatrix = glm::perspective(45.f, static_cast<float>(sys::window::width)/static_cast<float>(sys::window::height), 1.0f/128.0f, 1000.0f);

		prog.load("normal.frag", "PN.vert");

		prog.uniform("color", 1.0, 1.0, 1.0);

		{
			auto &&mesh = src::mem::mesh({
				 1.0f,  1.0f, 0.0f,
				 1.0f, -1.0f, 0.0f,
				-1.0f,  1.0f, 0.0f,
				-1.0f, -1.0f, 0.0f,
			});

			mesh->arrays.emplace_back(GL_TRIANGLE_STRIP, 0, 4);
			mesh->layouts.emplace_back(0, 3, GL_FLOAT, 0, 0);

			dummy.load(move(mesh));
		}

		{
			auto &&mesh= src::obj::mesh("suzanne.obj");

			suzanne.load(move(mesh));
		}

		{
			auto &&mesh= src::sbm::mesh("venus.sbm");

			venus.load(move(mesh));
		}

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
				cout << "Reloading shaders..." << endl;
				gl::Program::reloadAll();
				cout << "done" << endl;
			}

			if (sys::key::hit(KEY_F6))
			{
				cout << "Reloading meshes..." << endl;
				gl::Mesh::reloadAll();
				cout << "done" << endl;
			}

			if (sys::key::hit(KEY_F7))
			{
				cout << "Reloading fonts..." << endl;
				gl::Font::reloadAll();
				cout << "done" << endl;
			}

			if (sys::key::hit(KEY_F11))
			{
				cout << "Switching window mode..." << endl;
				currentMode = (currentMode + 1) % util::countOf(modes);
				sys::window::switchMode(modes[currentMode]);
				cout << "done" << endl;

				gl::init();

				cout << "Reloading meshes..." << endl;
				gl::Mesh::reloadAll();
				cout << "done" << endl;

				cout << "Reloading fonts..." << endl;
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


			glm::vec3 position_delta{0.0, 0.0, 0.0};
			glm::vec3 rotation_delta{0.0, 0.0, 0.0};

			position_delta.z = (sys::key::pressed('s') - sys::key::pressed('w'));
			position_delta.x = (sys::key::pressed('d') - sys::key::pressed('a'));
			position_delta.y = (sys::key::pressed(KEY_SPACE) - sys::key::pressed(KEY_CTRL));

			if (glm::length(position_delta) != 0.0)
			{
				position_delta = glm::normalize(position_delta);

				float speed = 10.f;
				position_delta *= speed * sys::dt;
			}

			camera.updateRecalc(glm::vec3{0.f}, position_delta);

			prog.use();
			prog.uniform("P", projectionMatrix);
			prog.uniform("V", camera.viewMatrix);

			prog.uniform("M", glm::rotate(glm::translate(glm::mat4{1.f}, glm::vec3{2.f, 0.f, 0.f}), static_cast<float>(90.0 * sys::time()), glm::vec3{0.f, 1.f, 0.f}));

			suzanne.render();

			prog.uniform("M", glm::rotate(glm::translate(glm::mat4{1.f}, glm::vec3{-2.f, 0.f, 0.f}), static_cast<float>(-90.0 * sys::time()), glm::vec3{0.f, 1.f, 0.f}));
			venus.render();

			{
				double ft = sys::time() - sys::ct;

				{
					ostringstream oss;
					oss << setprecision(4) << fixed;
					oss << "dt=" << sys::dt * 1000.0 << " ms\n";
					oss << "ft=" << ft * 1000.0 << " ms\n";
					oss << "fps=" << 1.0/sys::dt << "\n";
					oss << "fps=" << 1.0/ft << " (real)\n";

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