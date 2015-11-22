#include <pch.hpp>

#include "ssao.hpp"

#include <app/events.hpp>

#include <core/rn/mesh.hpp>
#include <core/rn/format.hpp>

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
	auto texZ = make_shared<rn::Tex2D>("fx::SSAO::fbZ.color[0]");
	texZ->width = win::internalWidth;
	texZ->height = win::internalHeight;
	texZ->mipLevels = zMipLevels;
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
	texAO->width = win::internalWidth;
	texAO->height = win::internalHeight;
	texAO->internalFormat = rn::format::RGB8.layout;
	texAO->reload();

	fbAO.clearColorValue = glm::vec4{1.f, 0.f, 0.f, 0.f};
	fbAO.attachColor(0, texAO);
	fbAO.reload();

	auto texBlur = make_shared<rn::Tex2D>("fx::SSAO::fbBlur.color[0]");
	texBlur->width = win::internalWidth;
	texBlur->height = win::internalHeight;
	texBlur->internalFormat = rn::format::RGB8.layout;
	texBlur->reload();

	fbBlur.attachColor(0, texBlur);
	fbBlur.reload();

	listenerWindowResize.attach([&] (const win::WindowResizeEvent &e)
	{
		clog << "WindowResizeEvent{" << e.width << ", " << e.height << "}" << endl;

		{
			auto texZ = dynamic_cast<rn::Tex2D *>(fbZ.color(0));
			texZ->width = win::internalWidth;
			texZ->height = win::internalHeight;
			texZ->reload();
		}

		fbZ.width = win::internalWidth;
		fbZ.height = win::internalHeight;
		fbZ.reload();

		for (GLsizei i = 0; i < zMipLevels; i++)
		{
			fbZMipMaps[i].width = win::internalWidth;
			fbZMipMaps[i].height = win::internalHeight;
			fbZMipMaps[i].reload();
		}

		{
			auto texAO = dynamic_cast<rn::Tex2D *>(fbAO.color(0));
			texAO->width = win::internalWidth;
			texAO->height = win::internalHeight;
			texAO->reload();
		}

		fbAO.width = win::internalWidth;
		fbAO.height = win::internalHeight;
		fbAO.reload();

		{
			auto texBlur = dynamic_cast<rn::Tex2D *>(fbBlur.color(0));
			texBlur->width = win::internalWidth;
			texBlur->height = win::internalHeight;
			texBlur->reload();
		}

		fbBlur.width = win::internalWidth;
		fbBlur.height = win::internalHeight;
		fbBlur.reload();
	});

	projection = projectionIn;

	listenerProjectionChanged.attach([&] (const ProjectionChangedEvent &e)
	{
		clog << "ProjectionChangedEvent" << endl;
		projection = e.projection;
	});

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

	// glm::vec3 clipInfo;

	// if (isinf(projection.zFar))
	// {
	// 	clipInfo = glm::vec3{projection.zNear, -1.f, 1.f};
	// }
	// else
	// {
	// 	clipInfo = glm::vec3{projection.zNear * projection.zFar, projection.zNear - projection.zFar, projection.zFar};
	// }

	// progReconstructZ.uniform("clipInfo", clipInfo);

	// const auto &P = projection.matrix;

	// glm::vec4 a = P * glm::vec4{0.5, 0.0, -1.0, 1.0};
	// a /= a.w;

	// GLfloat pixelScale = fbZ.width * a.x;

	// glm::vec4 projectionInfo{
	// 	-2.0 / (fbZ.width  * P[0][0]),
	// 	-2.0 / (fbZ.height * P[1][1]),
	// 	(1.0 - P[2][0]) / P[0][0],
	// 	(1.0 - P[2][1]) / P[1][1]
	// };

	// progSAO.uniform("P", projection.matrix);
	// progSAO.uniform("invP", projection.invMatrix);
	// progSAO.uniform("projectionInfo", projectionInfo);
	// progSAO.uniform("pixelScale", pixelScale);
	// progSAO.uniform("intensity", intensity);
	// progSAO.uniform("radius", radius);
	// progSAO.uniform<GLint>("zMipLevels", zMipLevels);

	profZ.init();
	profMipMaps.init();
	profAO.init();
	profBlur.init();

	dirty = true;
}

void SSAO::clear()
{
	if ( ! dirty)
	{
		return;
	}

	fbZ.clear(GL_COLOR_BUFFER_BIT);


	for (GLsizei i = 0; i < zMipLevels; i++)
	{
		auto &fb = fbZMipMaps[i];
		fb.clear(GL_COLOR_BUFFER_BIT);
	}

	fbAO.clear(GL_COLOR_BUFFER_BIT);

	fbBlur.clear(GL_COLOR_BUFFER_BIT);

	dirty = false;
}

void SSAO::genMipMaps(const rn::Tex &texDepth)
{
	RN_SCOPE_DISABLE(GL_BLEND);
	RN_SCOPE_DISABLE(GL_DEPTH_TEST);
	RN_SCOPE_DISABLE(GL_STENCIL_TEST);

	glm::vec3 clipInfo{projection.zNear, -1.f, 1.f};
	if ( ! isinf(projection.zFar))
	{
		clipInfo = glm::vec3{projection.zNear * projection.zFar, projection.zNear - projection.zFar, projection.zFar};
	}

	profZ.start();

	{
		RN_FB_BIND(fbZ);
		fbZ.clear(GL_COLOR_BUFFER_BIT);

		progReconstructZ.use();

		progReconstructZ.uniform("clipInfo", clipInfo);
		progReconstructZ.var("texDepth", texDepth.bind(0));

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

	dirty = true;
}

void SSAO::computeAO(const rn::Tex &texNormal)
{
	profAO.start();

	RN_SCOPE_DISABLE(GL_DEPTH_TEST);
	RN_SCOPE_DISABLE(GL_BLEND);

	RN_FB_BIND(fbAO);
	fbAO.clear(rn::BUFFER_COLOR);

	const auto &P = projection.matrix;

	glm::vec4 a = P * glm::vec4{0.5, 0.0, -1.0, 1.0};
	a /= a.w;

	GLfloat pixelScale = fbZ.width * a.x;

	glm::vec4 projectionInfo{
		-2.0 / (fbZ.width  * P[0][0]),
		-2.0 / (fbZ.height * P[1][1]),
		(1.0 - P[2][0]) / P[0][0],
		(1.0 - P[2][1]) / P[1][1]
	};

	progSAO.use();
	progSAO.uniform("projectionInfo", projectionInfo);
	progSAO.uniform("pixelScale", pixelScale);
	progSAO.uniform("intensity", intensity);
	progSAO.uniform("radius", radius);
	progSAO.uniform<GLint>("zMipLevels", zMipLevels);

	size_t unit = 0;
	progSAO.var("texZ", fbZ.color(0)->bind(unit++));
	progSAO.var("texNormal", texNormal.bind(unit++));

	rn::Mesh::quad.render();

	progSAO.forgo();

	profAO.stop();

	dirty = true;
}

void SSAO::blur()
{
	profBlur.start();

	RN_SCOPE_DISABLE(GL_DEPTH_TEST);
	RN_SCOPE_DISABLE(GL_BLEND);

	progBlur.use();

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

	dirty = true;
}

} // fx