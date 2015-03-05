#include "ssao.hpp"
#include <core/rn/mesh.hpp>
#include <core/rn/format.hpp>
#include <core/ngn/window.hpp>
#include <string>
#include <cmath>
#include <cassert>
#include <limits>
#include <memory>
#include <iostream>

namespace fx
{

using namespace std;
namespace win = ngn::window;

void SSAO::init(const comp::Projection &projectionIn)
{
	projection = projectionIn;

	auto texZ = make_shared<rn::Tex2D>("fx::SSAO::fbZ.color[0]");

	texZ->width = win::width;
	texZ->height = win::height;
	texZ->levels = zMipLevels;
	texZ->minFilter = rn::MIN_NEAREST_NEAREST;
	texZ->magFilter = rn::MAG_LINEAR;
	texZ->wrapS = rn::WRAP_CLAMP;
	texZ->wrapT = rn::WRAP_CLAMP;
	texZ->internalFormat = rn::format::R32F.layout;
	texZ->reload();

	fbZ.attachColor(0, texZ);
	fbZ.reload();

	fbZMipMaps.resize(zMipLevels);

	for (GLsizei i = 0; i < zMipLevels; i++)
	{
		fbZMipMaps[i].fbName = "fx::SSAO::fbZMipMaps[" + to_string(i) + "]";
		fbZMipMaps[i].attachColor(0, texZ, i + 1);
		fbZMipMaps[i].reload();
	}

	auto texAO = make_shared<rn::Tex2D>("fx::SSAO::fbAO.color[0]");
	texAO->width = win::width;
	texAO->height = win::height;
	texAO->internalFormat = rn::format::RGB8.layout;
	texAO->reload();

	fbAO.clearColor = glm::vec4{1.f, 0.f, 0.f, 0.f};
	fbAO.attachColor(0, texAO);
	fbAO.reload();

	auto texBlur = make_shared<rn::Tex2D>("fx::SSAO::fbBlur.color[0]");
	texBlur->width = win::width;
	texBlur->height = win::height;
	texBlur->internalFormat = rn::format::RGB8.layout;
	texBlur->reload();

	fbBlur.attachColor(0, texBlur);
	fbBlur.reload();

	try
	{
		progReconstructZ.load("fx/ssao/reconstructZ.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progMinify.load("fx/ssao/minify.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

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

	glm::vec3 clipInfo;

	if (isinf(projection.zFar))
	{
		clipInfo = glm::vec3{projection.zNear, -1.f, 1.f};
	}
	else
	{
		clipInfo = glm::vec3{projection.zNear * projection.zFar, projection.zNear - projection.zFar, projection.zFar};
	}

	progReconstructZ.uniform("clipInfo", clipInfo);

	const auto &P = projection.matrix;

	glm::vec4 a = P * glm::vec4{0.5, 0.0, -1.0, 1.0};
	a /= a.w;

	GLfloat pixelScale = fbZ.width * a.x;

	glm::vec4 projectionInfo{
		-2.0 / (fbZ.width  * P[0].x),
		-2.0 / (fbZ.height * P[1].y),
		(1.0 - P[2].x) / P[0].x,
		(1.0 + P[2].y) / P[1].y
	};

	progSAO.uniform("zFar", projection.zFar);
	progSAO.uniform("projectionInfo", projectionInfo);
	progSAO.uniform("pixelScale", pixelScale);
	progSAO.uniform("intensity", intensity);
	progSAO.uniform("radius", radius);
	progSAO.uniform<GLint>("zMipLevels", zMipLevels);

	profZ.init();
	profMipMaps.init();
	profAO.init();
	profBlur.init();
}

void SSAO::clear()
{
	{
		RN_FB_BIND(fbZ);
		fbZ.clear(GL_COLOR_BUFFER_BIT);
	}

	for (GLsizei i = 0; i < zMipLevels; i++)
	{
		auto &fb = fbZMipMaps[i];
		RN_FB_BIND(fb);

		fb.clear(GL_COLOR_BUFFER_BIT);
	}

	{
		RN_FB_BIND(fbAO);
		fbAO.clear(GL_COLOR_BUFFER_BIT);
	}

	{
		RN_FB_BIND(fbBlur);
		fbBlur.clear(GL_COLOR_BUFFER_BIT);
	}
}

void SSAO::genMipMaps(rn::FB &fbGBuffer)
{
	RN_SCOPE_DISABLE(GL_BLEND);
	RN_SCOPE_DISABLE(GL_DEPTH_TEST);
	RN_SCOPE_DISABLE(GL_STENCIL_TEST);

	profZ.start();

	{
		RN_FB_BIND(fbZ);
		fbZ.clear(GL_COLOR_BUFFER_BIT);

		progReconstructZ.use();

		progReconstructZ.var("texDepth", fbGBuffer.depth()->bind(0));

		rn::Mesh::quad.render();

		progReconstructZ.forgo();
	}

	profZ.stop();

	auto texZ = fbZ.color(0);

	assert(texZ != nullptr);

	profMipMaps.start();

	{
		progMinify.use();
		progMinify.var("texZ", fbZ.color(0)->bind(0));

		for (GLsizei i = 0; i < zMipLevels; i++)
		{
			RN_FB_BIND(fbZMipMaps[i]);

			progMinify.var("prevLevel", i);

			rn::Mesh::quad.render();
		}

		progMinify.forgo();
	}

	profMipMaps.stop();
}

void SSAO::computeAO(rn::FB &fbGBuffer)
{
	profAO.start();

	RN_SCOPE_DISABLE(GL_DEPTH_TEST);
	RN_SCOPE_DISABLE(GL_BLEND);

	RN_FB_BIND(fbAO);
	fbAO.clear(GL_COLOR_BUFFER_BIT);

	progSAO.use();

	progSAO.var("texColor", fbGBuffer.color(0)->bind(0));
	progSAO.var("texNormal", fbGBuffer.color(1)->bind(1));
	progSAO.var("texDepth", fbGBuffer.depth()->bind(2));
	progSAO.var("texZ", fbZ.color(0)->bind(4));

	rn::Mesh::quad.render();

	progSAO.forgo();

	profAO.stop();
}

void SSAO::blur(rn::FB &fbGBuffer)
{
	profBlur.start();

	RN_SCOPE_DISABLE(GL_DEPTH_TEST);
	RN_SCOPE_DISABLE(GL_BLEND);

	progBlur.use();
	progBlur.var("texNormal", fbGBuffer.color(1)->bind(1));
	progBlur.var("texDepth", fbGBuffer.depth()->bind(2));

	{
		RN_FB_BIND(fbBlur);


		progBlur.var("texSource", fbAO.color(0)->bind(0));
		progBlur.var("scale", glm::vec2{1.f, 0.f});

		rn::Mesh::quad.render();
	}

	{
		RN_FB_BIND(fbAO);

		progBlur.var("texSource", fbBlur.color(0)->bind(0));
		progBlur.var("scale", glm::vec2{0.f, 1.f});

		rn::Mesh::quad.render();
	}

	progBlur.forgo();

	profBlur.stop();
}

} // fx