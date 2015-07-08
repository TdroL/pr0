#include <array>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <core/rn.hpp>
#include <core/rn/ext.hpp>
#include <core/rn/fb.hpp>
#include <core/rn/font.hpp>
// #include <core/rn/mesh.hpp>
// #include <core/rn/program.hpp>
// #include <core/util.hpp>
#include <core/util/align.hpp>
#include <core/util/count.hpp>
#include <core/util/scope.hpp>
// #include <core/util/initq.hpp>
#include <core/util/toggle.hpp>
#include <core/ngn.hpp>
#include <core/ngn/fs.hpp>
#include <core/ngn/key.hpp>
#include <core/ngn/ino.hpp>
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
#if defined(DEBUG) || defined(_DEBUG)
	cout << "Debug: ON" << endl;
#else
	cout << "Debug: OFF" << endl;
#endif

	try
	{
		ngn::ino::init(argc, argv);

		ngn::init();

		UTIL_SCOPE_EXIT([] ()
		{
			ngn::deinit();
		});


		UTIL_DEBUG
		{
			if (ngn::ino::has("--help"))
			{
				clog << "Options:" << endl;
				clog << "  --print-exts    - shows core extensions support" << endl;
				clog << "  --frames=<num>  - limits number of rendered frames" << endl;
				clog << "  --log-gl-calls  - logs every OpenGL call" << endl;

				return EXIT_SUCCESS;
			}

			if (ngn::ino::has("--print-exts"))
			{
				clog << "Extensions:" << endl;
				clog << rn::getExtensionsInfo() << endl;
				clog << endl;
				clog << "Supported extensions:" << endl;
				clog << boolalpha;
				for (auto *ext : rn::ext::list)
				{
					clog << ext->name << " = " << (bool) *ext << endl;
				}
				clog << noboolalpha;

				return EXIT_SUCCESS;
			}
		}

		App app{};
		app.init();

		rn::Font font{"DejaVuSansMono"};
		font.load("DejaVu/DejaVuSansMono.ttf");

		const win::Mode modes[]
		{
			win::Mode::windowed,
			win::Mode::borderless,
			win::Mode::fullscreen
		};

		const string modeNames[]
		{
			"windowed",
			"borderless",
			"fullscreen",
		};

		size_t currentMode = 0;

		const int vsyncs[]
		{
			-1, // progressive
			 0, // off
			 1, // on
		};

		const string vsyncNames[]
		{
			"progressive",
			"off",
			"on",
		};

		size_t currentVsync = 0;

		/* Test: switch to window mode */
		UTIL_DEBUG
		{
			currentMode = 0;
			currentVsync = 1;

			clog << "Test: switching to " << modeNames[currentMode] << " " << vsyncNames[currentVsync] << endl;

			win::switchMode(modes[currentMode], vsyncs[currentVsync]);
			rn::reloadSoftAll();
		}

		/* Refresh system */

		ngn::update();

		for (int frame = 0, frames = ngn::ino::get("--frames", -1); (frames == -1 || frame < frames) && ! win::shouldClose(); frame++)
		{
			NGN_LOOP;

			if (key::hit(KEY_ESCAPE))
			{
				win::close();
			}

			if (key::hit(KEY_F4))
			{
				cout << "Reloading scene..." << endl;

				try
				{
					app.scene.reload();
					cout << "done" << endl;
				}
				catch (const string &e)
				{
					cerr << e << endl;
				}
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
				cout << "Reloading FBs..." << endl;
				rn::FB::reloadAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F9))
			{
				cout << "Reloading textures..." << endl;
				rn::Tex2D::reloadAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F10))
			{
				currentVsync = (currentVsync + 1) % util::countOf(vsyncs);

				cout << "Switching vsync mode to \"" << vsyncNames[currentVsync] << "\" (" << vsyncs[currentVsync] << ") ..." << endl;
				win::switchMode(modes[currentMode], vsyncs[currentVsync]);
				rn::reloadSoftAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_F11))
			{
				cout << "Switching window mode..." << endl;
				currentMode = (currentMode + 1) % util::countOf(modes);
				win::switchMode(modes[currentMode], vsyncs[currentVsync]);
				cout << "done" << endl;

				cout << "Soft reloading GL..." << endl;
				rn::reloadSoftAll();
				cout << "done" << endl;
			}

			if (key::hit(KEY_ESCAPE))
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
					oss << "dt=" << ngn::dt * 1000.0 << "ms\n";
					oss << "ft=" << ft * 1000.0 << "ms\n";
					oss << "fps=" << 1.0 / ngn::dt << "\n";
					oss << "fps=" << 1.0 / ft << " (frame)\n";
					oss << "triangles=" << rn::stats.triangles << "\n";
					oss << "\n";

					oss << "render=" << app.profRender.ms() << "ms (" << 1000.0 / app.profRender.ms() << ")\n";
					oss << "  GBuffer=" << app.profGBuffer.ms() << "ms\n";
					oss << "  DirectionalLight=" << app.profDirectionalLight.ms() << "ms\n";
					oss << "  PointLight=" << app.profPointLight.ms() << "ms\n";
					oss << "  FlatLight=" << app.profFlatLight.ms() << "ms\n";
					oss << "  SSAO=" << app.profSSAO.ms() << "ms\n";
					oss << "    Z=" << app.ssao.profZ.ms() << "ms\n";
					oss << "    MipMaps=" << app.ssao.profMipMaps.ms() << "ms\n";
					oss << "    AO=" << app.ssao.profAO.ms() << "ms\n";
					oss << "    Blur=" << app.ssao.profBlur.ms() << "ms\n";
					oss << "  CSM=???ms\n";
					oss << "    Render=" << app.csm.profRender.ms() << "ms\n";
					oss << "    Blur=" << app.csm.profBlur.ms() << "ms\n";

					oss << "\n";
					oss << "F4 - reload scene\n";
					oss << "F5 - reload shaders\n";
					oss << "F6 - reload meshes\n";
					oss << "F7 - reload fonts\n";
					oss << "F8 - reload FBOs\n";
					oss << "F9 - reload textures\n";
					oss << "F10 - change vsync mode (current: " << vsyncNames[currentVsync] << ")\n";
					oss << "F11 - change window mode (current: " << modeNames[currentMode] << ")\n";
					oss << "\n";
					oss << "Movement: W, A, S, D\n";
					oss << "Camera: arrows\n";
					oss << "Point Light: Keypad 8, 4, 5, 6\n\n";
					oss << "Toggles:\n";

					for (auto &toggle : util::Toggle::collection)
					{
						oss << "  " << toggle->toggleName << " = " << toggle->value << "\n";
					}

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