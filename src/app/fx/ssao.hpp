#ifndef APP_FX_SSAO_HPP
#define APP_FX_SSAO_HPP

#include <core/rn.hpp>
#include <core/rn/fbo.hpp>
#include <core/rn/program.hpp>

namespace app
{

namespace fx
{

class SSAO
{
public:
	GLfloat radius = 0.f;
	GLfloat bias = 0.f;
	GLfloat intensity = 0.f;

	GLsizei zMipLevels = 5;

	rn::FBO fboZ{"app::fx::SSAO::fboZ"};
	rn::FBO fboBuffer{"app::fx::SSAO::fboBuffer"};

	rn::Program progSAO{};
	rn::Program progBlur{};

	void init();
};

} // fx

} // app

#endif