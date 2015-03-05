#include "ssao.hpp"
#include <core/ngn/window.hpp>
#include <string>

namespace app
{

namespace fx
{

using namespace std;
namespace win = ngn::window;

void SSAO::init()
{
	fboZ.width = win::width;
	fboZ.height = win::height;
	fboZ.clearColor = glm::vec4{1.f};
	fboZ.setColorTex(GL_R32F, 0, 5);
	fboZ.create();

	fboBuffer.width = win::width;
	fboBuffer.height = win::height;
	fboBuffer.clearColor = glm::vec4{1.f};
	fboBuffer.setColorTex(GL_RGB8, 0); // AO + packed Z
	fboBuffer.create();

	try
	{
		progSAO.load("fx/ssao/sao.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progBlur.load("fx/ssao/blur.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}
}

} // fx

} // app
