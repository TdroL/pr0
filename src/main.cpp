#include <array>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <core/rn.hpp>
#include <core/rn/ext.hpp>
// #include <core/rn/fb.hpp>
// #include <core/rn/font.hpp>
// #include <core/rn/mesh.hpp>
// #include <core/rn/program.hpp>
// #include <core/util.hpp>
// #include <core/util/align.hpp>
// #include <core/util/count.hpp>
#include <core/util/scope.hpp>
// #include <core/util/initq.hpp>
// #include <core/util/toggle.hpp>
#include <core/ngn.hpp>
// #include <core/ngn/fs.hpp>
// #include <core/ngn/key.hpp>
#include <core/ngn/ino.hpp>
#include <core/ngn/loop.hpp>
#include <core/ngn/window.hpp>
// #include <core/src/mem.hpp>
// #include <core/src/sbm.hpp>

#include "app.hpp"

using namespace std;

// namespace fs = ngn::fs;
// namespace key = ngn::key;
namespace win = ngn::window;

int main(int argc, char const* argv[])
{

	UTIL_DEBUG
	{
		cout << "Debug: ON" << endl;
	}
	else
	{
		cout << "Debug: OFF" << endl;
	}

	try
	{
		ngn::ino::init(argc, argv);

		UTIL_DEBUG
		{
			if (ngn::ino::has("--help"))
			{
				clog << "Options:" << endl;
				clog << "  --print-exts    - shows core extensions support" << endl;
				clog << "  --frames=<num>  - limits number of rendered frames" << endl;
				clog << "  --log-gl-calls  - logs every OpenGL call" << endl;

				exit(EXIT_SUCCESS);
			}

			if (ngn::ino::has("--print-exts"))
			{
				ngn::initQ().attachFirst([&] ()
				{
					clog << "Extensions:" << endl;
					clog << rn::getExtensionsInfo() << endl;
					clog << endl;
					clog << "Supported extensions:" << endl;
					clog << boolalpha;
					for (auto *ext : rn::ext::list)
					{
						clog << ext->name << " = " << (bool) (*ext) << endl;
					}
					clog << noboolalpha;

					exit(EXIT_SUCCESS);
				});

				// clog << "Extensions:" << endl;
				// clog << rn::getExtensionsInfo() << endl;
				// clog << endl;
				// clog << "Supported extensions:" << endl;
				// clog << boolalpha;
				// for (auto *ext : rn::ext::list)
				// {
				// 	clog << ext->name << " = " << (bool) (*ext) << endl;
				// }
				// clog << noboolalpha;

				// return EXIT_SUCCESS;
			}
		}

		ngn::init();

		UTIL_SCOPE_EXIT([] ()
		{
			ngn::deinit();
		});

		App app{};
		app.init();

		/* Refresh system */

		ngn::update();

		for (int frame = 0, frames = ngn::ino::get("--frames", -1); (frames == -1 || frame < frames) && ! win::shouldClose(); frame++)
		{
			NGN_LOOP;

			app.update();

			app.render();
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