#define GLM_SWIZZLE

#include "csm.hpp"

#include <app/comp/boundingobject.hpp>
#include <app/comp/directionallight.hpp>
#include <app/comp/mesh.hpp>
#include <app/comp/occluder.hpp>
#include <app/comp/position.hpp>
#include <app/comp/projection.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/view.hpp>
#include <app/proc/frustumprocess.hpp>
#include <app/proc/meshrenderer.hpp>

#include <core/phs/frustum.hpp>
#include <core/rn/mesh.hpp>
#include <core/util/count.hpp>

#include <iostream>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/string_cast.hpp>

namespace fx
{

using namespace std;
using namespace comp;

enum CORNERS {
	RTN = 0,
	LTN = 1,
	RBN = 2,
	LBN = 3,
	RTF = 4,
	LTF = 5,
	RBF = 6,
	LBF = 7
};

void CSM::init()
{
	cascades.resize(splits.size());
	fbShadows.resize(splits.size());
	Ps.resize(splits.size());

	texCascades = make_shared<rn::Tex2DArray>("fx::CSM::texCascades");
	texCascades->width = 1024;
	texCascades->height = 1024;
	texCascades->size = splits.size();
	texCascades->internalFormat = rn::format::RGBA32F.layout;
	texCascades->reload();

	texDepths = make_shared<rn::Tex2DArray>("fx::CSM::texDepths");
	texDepths->width = texCascades->width;
	texDepths->height = texCascades->height;
	texDepths->size = splits.size();
	texDepths->internalFormat = rn::format::D32F.layout;
	texDepths->reload();

	for (size_t i = 0; i < fbShadows.size(); i++)
	{
		auto &fb = fbShadows[i];

		fb.fbName = "fx::CSM::fbShadows[" + to_string(i) + "]";

		fb.attachColor(0, texCascades, i);
		fb.attachDepth(texDepths, i);

		fb.clearColor = glm::vec4{1.f};
		fb.reload();
	}

	auto texBlurBuffer = make_shared<rn::Tex2D>("fx::CSM::texBlurBuffer");

	texBlurBuffer->width = texCascades->width;
	texBlurBuffer->height = texCascades->height;
	texBlurBuffer->internalFormat = texCascades->internalFormat;
	texBlurBuffer->reload();

	fbBlurBuffer.fbName = "fx::CSM::fbBlurBuffer";
	fbBlurBuffer.attachColor(0, texBlurBuffer);
	fbBlurBuffer.reload();

	try
	{
		progDepth.load("fx/csm/depth.frag", "P.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progBlurH.load("fx/csm/blurH.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		progBlurV.load("fx/csm/blurV.frag", "rn/fbo.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	profRender.init();
	profBlur.init();
}

void CSM::setup(const ecs::Entity &lightId)
{
	const auto &light = ecs::get<DirectionalLight>(lightId);
	const auto &cameraPosition = ecs::get<Position>(cameraId).position;
	const auto &projection = ecs::get<Projection>(cameraId);
	const auto &view = ecs::get<View>(cameraId);

	glm::vec4 frustumCorners[8];
	buildCorners(projection.matrix * view.matrix, frustumCorners);

	V = glm::lookAt(cameraPosition, cameraPosition - light.direction, glm::vec3{0.f, 1.f, 0.f});

	glm::vec4 frustumSplitCorners[] = {
		frustumCorners[RTN],
		frustumCorners[LTN],
		frustumCorners[RBN],
		frustumCorners[LBN],
		frustumCorners[RTF],
		frustumCorners[LTF],
		frustumCorners[RBF],
		frustumCorners[LBF]
	};

	float zMax = findSceneZMax(lightId);

	frustumSplitCorners[RTF] = frustumSplitCorners[RTN];
	frustumSplitCorners[LTF] = frustumSplitCorners[LTN];
	frustumSplitCorners[RBF] = frustumSplitCorners[RBN];
	frustumSplitCorners[LBF] = frustumSplitCorners[LBN];

	for (size_t i = 0; i < splits.size(); i++)
	{
		frustumSplitCorners[RTN] = frustumSplitCorners[RTF];
		frustumSplitCorners[LTN] = frustumSplitCorners[LTF];
		frustumSplitCorners[RBN] = frustumSplitCorners[RBF];
		frustumSplitCorners[LBN] = frustumSplitCorners[LBF];
		frustumSplitCorners[RTF] = glm::lerp(frustumCorners[RTN], frustumCorners[RTF], splits[i]);
		frustumSplitCorners[LTF] = glm::lerp(frustumCorners[LTN], frustumCorners[LTF], splits[i]);
		frustumSplitCorners[RBF] = glm::lerp(frustumCorners[RBN], frustumCorners[RBF], splits[i]);
		frustumSplitCorners[LBF] = glm::lerp(frustumCorners[LBN], frustumCorners[LBF], splits[i]);

		cascades[i] = -glm::lerp(projection.zNear, projection.zFar, splits[i]);

		Ps[i] = buildShadowPMatrix(frustumSplitCorners, zMax);
	}
}

void CSM::render()
{
	profRender.start();

	RN_SCOPE_DISABLE(GL_BLEND);
	RN_SCOPE_DISABLE(GL_STENCIL_TEST);

	progDepth.use();

	progDepth.uniform("V", V);

	for (size_t i = 0; i < Ps.size(); i++)
	{
		const auto &P = Ps[i];
		progDepth.uniform("P", P);

		auto &fbShadow = fbShadows[i];

		RN_FB_BIND(fbShadow);

		fbShadow.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH);

		const phs::Frustum boundingFrustum{P * V};

		for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
		{
			if (ecs::has<BoundingObject>(entity) && ! proc::FrustumProcess::isVisible(entity, boundingFrustum))
			{
				continue;
			}

			proc::MeshRenderer::render(entity, progDepth);
		}
	}

	progDepth.forgo();

	profRender.stop();

	profBlur.start();

	RN_SCOPE_DISABLE(GL_DEPTH_TEST);

	for (size_t i = 0; i < fbShadows.size() - 1; i++)
	{
		auto &fbShadow = fbShadows[i];

		{
			RN_FB_BIND(fbBlurBuffer);

			fbBlurBuffer.clear(rn::BUFFER_COLOR);

			progBlurH.use();
			// progBlurH.var("texSource", fbShadow.color(0)->bind(0));
			progBlurH.var("texSource", texCascades->bind(0));
			progBlurH.var("layer", static_cast<GLint>(i));

			rn::Mesh::quad.render();
			progBlurH.forgo();
		}

		{
			RN_FB_BIND(fbShadow);

			fbShadow.clear(rn::BUFFER_COLOR);

			progBlurV.use();
			progBlurV.var("texSource", fbBlurBuffer.color(0)->bind(0));
			progBlurV.var("layer", static_cast<GLint>(i));

			rn::Mesh::quad.render();
			progBlurV.forgo();
		}
	}

	profBlur.stop();
}

void CSM::buildCorners(const glm::mat4 &VP, glm::vec4 (&output)[8])
{
	const auto invVP = glm::inverse(VP);

	output[RTN] = invVP * glm::vec4{ 1.f,  1.f, -1.f, 1.f};
	output[LTN] = invVP * glm::vec4{-1.f,  1.f, -1.f, 1.f};
	output[RBN] = invVP * glm::vec4{ 1.f, -1.f, -1.f, 1.f};
	output[LBN] = invVP * glm::vec4{-1.f, -1.f, -1.f, 1.f};
	output[RTF] = invVP * glm::vec4{ 1.f,  1.f,  1.f, 1.f};
	output[LTF] = invVP * glm::vec4{-1.f,  1.f,  1.f, 1.f};
	output[RBF] = invVP * glm::vec4{ 1.f, -1.f,  1.f, 1.f};
	output[LBF] = invVP * glm::vec4{-1.f, -1.f,  1.f, 1.f};

	output[RTN] /= output[RTN].w;
	output[LTN] /= output[LTN].w;
	output[RBN] /= output[RBN].w;
	output[LBN] /= output[LBN].w;
	output[RTF] /= output[RTF].w;
	output[LTF] /= output[LTF].w;
	output[RBF] /= output[RBF].w;
	output[LBF] /= output[LBF].w;
}

float CSM::findSceneZMax(const ecs::Entity &lightId)
{
	glm::vec3 lightDirection = glm::normalize(ecs::get<DirectionalLight>(lightId).direction);
	float zMax = -numeric_limits<float>::max();

	for (auto &entity : ecs::findWith<Transform, Mesh, BoundingObject, Occluder>())
	{
		auto &boundingObject = ecs::get<BoundingObject>(entity);

		float dist = glm::dot(lightDirection, boundingObject.sphere.pos) + boundingObject.sphere.radius;

		zMax = max(zMax, dist);
	}

	return zMax;
}

glm::mat4 CSM::buildShadowPMatrix(const glm::vec4 (&corners)[8], float zMax)
{
	glm::vec4 shadowFrustumCorners[8] = {
		V * corners[RTN],
		V * corners[LTN],
		V * corners[RBN],
		V * corners[LBN],
		V * corners[RTF],
		V * corners[LTF],
		V * corners[RBF],
		V * corners[LBF]
	};

	float shadowFrustumLeft = shadowFrustumCorners[0].x;
	float shadowFrustumRight = shadowFrustumCorners[0].x;
	float shadowFrustumBottom = shadowFrustumCorners[0].y;
	float shadowFrustumTop = shadowFrustumCorners[0].y;
	float shadowFrustumZNear = shadowFrustumCorners[0].z;
	float shadowFrustumZFar = shadowFrustumCorners[0].z;

	for (size_t i = 1; i < 8; i++)
	{
		shadowFrustumLeft = min(shadowFrustumLeft, shadowFrustumCorners[i].x);
		shadowFrustumRight = max(shadowFrustumRight, shadowFrustumCorners[i].x);
		shadowFrustumBottom = min(shadowFrustumBottom, shadowFrustumCorners[i].y);
		shadowFrustumTop = max(shadowFrustumTop, shadowFrustumCorners[i].y);
		shadowFrustumZNear = max(shadowFrustumZNear, shadowFrustumCorners[i].z);
		shadowFrustumZFar = min(shadowFrustumZFar, shadowFrustumCorners[i].z);
	}

	shadowFrustumZNear = max(shadowFrustumZNear, zMax);

	return glm::ortho(shadowFrustumLeft, shadowFrustumRight, shadowFrustumBottom, shadowFrustumTop, -shadowFrustumZNear, -shadowFrustumZFar);
}

} // fx