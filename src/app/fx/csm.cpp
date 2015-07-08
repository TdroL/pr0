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

#include <core/phs/aabb.hpp>
#include <core/phs/frustum.hpp>
#include <core/rn/mesh.hpp>
#include <core/util/count.hpp>

#include <iostream>
#include <cmath>
#include <utility>
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
	Vs.resize(splits.size());

	texDepths = make_shared<rn::Tex2DArray>("fx::CSM::texDepths");
	texDepths->width = shadowResolution;
	texDepths->height = shadowResolution;
	texDepths->size = splits.size();
	texDepths->minFilter = rn::MinFilter::MIN_LINEAR;
	texDepths->magFilter = rn::MagFilter::MAG_LINEAR;
	texDepths->compareFunc = rn::CompareFunc::COMPARE_LEQUAL;
	texDepths->internalFormat = rn::format::D32F.layout;
	texDepths->reload();

	// texCascades = make_shared<rn::Tex2DArray>("fx::CSM::texCascades");
	// texCascades->width = texDepths->width;
	// texCascades->height = texDepths->height;
	// texCascades->size = splits.size();
	// texCascades->internalFormat = rn::format::RGBA32F.layout;
	// texCascades->reload();

	for (size_t i = 0; i < fbShadows.size(); i++)
	{
		auto &fb = fbShadows[i];

		fb.fbName = "fx::CSM::fbShadows[" + to_string(i) + "]";

		// fb.attachColor(0, texCascades, i);
		fb.attachDepth(texDepths, i);

		fb.clearColor = glm::vec4{1.f};
		fb.reload();
	}

	// auto texBlurBuffer = make_shared<rn::Tex2D>("fx::CSM::texBlurBuffer");

	// texBlurBuffer->width = texCascades->width;
	// texBlurBuffer->height = texCascades->height;
	// texBlurBuffer->internalFormat = texCascades->internalFormat;
	// texBlurBuffer->reload();

	// fbBlurBuffer.fbName = "fx::CSM::fbBlurBuffer";
	// fbBlurBuffer.attachColor(0, texBlurBuffer);
	// fbBlurBuffer.reload();

	try
	{
		progDepth.load("fx/csm/depth.frag", "P.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	// try
	// {
	// 	progBlurH.load("fx/csm/blurH.frag", "rn/fbo.vert");
	// }
	// catch (const string &e)
	// {
	// 	cerr << "Warning: " << e << endl;
	// }

	// try
	// {
	// 	progBlurV.load("fx/csm/blurV.frag", "rn/fbo.vert");
	// }
	// catch (const string &e)
	// {
	// 	cerr << "Warning: " << e << endl;
	// }

	profRender.init();
	profBlur.init();
}

void CSM::setup(const ecs::Entity &lightId)
{
	const auto &light = ecs::get<DirectionalLight>(lightId);

	const auto &projection = ecs::get<Projection>(cameraId);
	const auto &view = ecs::get<View>(cameraId);
	const auto &cameraPosition = ecs::get<Position>(cameraId).position;

	const float zNear = projection.zNear;
	const float zFar = projection.zFar;
	const float fovy = projection.fovy;
	const float aspect = projection.aspect;

	const auto lightDirection = glm::normalize(light.direction);
	glm::vec2 zMinMax = findSceneZMinMax(lightDirection);

	size_t splitCount = fbShadows.size();

	vector<float> splitFar(splitCount, zFar);
	vector<float> splitNear(splitCount, zNear);

	float lambda = .8f;
	float j = 1.f;

	// cout << "frustum=[" << zNear << " - " << zFar << "]" << endl;

	for (size_t i = 0; i < splitCount - 1; i++)
	{
		// splitFar[i] = glm::mix(
		// 	zNear + (j / (float) splitCount) * (zFar - zNear),
		// 	zNear * pow(zFar / zNear, j / (float) splitCount),
		// 	lambda
		// );
		splitFar[i] = glm::mix(zNear, zFar, splits[i]);

		splitNear[i + 1] = splitFar[i];

		j += 1.f;
	}

	// for (size_t i = 0; i < splitCount; i++) {
	// 	cout << "split[" << i << "]: [" << splitNear[i] << " - " << splitFar[i] << "]" << endl;
	// }

	// const auto V = glm::lookAt(cameraPosition, cameraPosition - light.direction, glm::vec3{0.f, 1.f, 0.f});
	const auto unitVec = projection.matrix * glm::vec4{1.f};

	// glm::vec4 corners[8];

	// corners[0] = projection.invMatrix * glm::vec4{ 1.f,  1.f, -1.f, 1.f};
	// corners[1] = projection.invMatrix * glm::vec4{-1.f,  1.f, -1.f, 1.f};
	// corners[2] = projection.invMatrix * glm::vec4{ 1.f, -1.f, -1.f, 1.f};
	// corners[3] = projection.invMatrix * glm::vec4{-1.f, -1.f, -1.f, 1.f};
	// corners[4] = projection.invMatrix * glm::vec4{ 1.f,  1.f,  1.f, 1.f};
	// corners[5] = projection.invMatrix * glm::vec4{-1.f,  1.f,  1.f, 1.f};
	// corners[6] = projection.invMatrix * glm::vec4{ 1.f, -1.f,  1.f, 1.f};
	// corners[7] = projection.invMatrix * glm::vec4{-1.f, -1.f,  1.f, 1.f};

	// corners[0] /= corners[0].w;
	// corners[1] /= corners[1].w;
	// corners[2] /= corners[2].w;
	// corners[3] /= corners[3].w;
	// corners[4] /= corners[4].w;
	// corners[5] /= corners[5].w;
	// corners[6] /= corners[6].w;
	// corners[7] /= corners[7].w;

	// cout << "corners[0]=" << glm::to_string(corners[0]) << endl;
	// cout << "corners[1]=" << glm::to_string(corners[1]) << endl;
	// cout << "corners[2]=" << glm::to_string(corners[2]) << endl;
	// cout << "corners[3]=" << glm::to_string(corners[3]) << endl;
	// cout << "corners[4]=" << glm::to_string(corners[4]) << endl;
	// cout << "corners[5]=" << glm::to_string(corners[5]) << endl;
	// cout << "corners[6]=" << glm::to_string(corners[6]) << endl;
	// cout << "corners[7]=" << glm::to_string(corners[7]) << endl;

	// cout << "cameraPosition=" << glm::to_string(cameraPosition) << endl;

	// const auto invVP = glm::inverse(projection.matrix * view.matrix);

	// corners[0] = invVP * glm::vec4{ 1.f,  1.f, -1.f, 1.f};
	// corners[1] = invVP * glm::vec4{-1.f,  1.f, -1.f, 1.f};
	// corners[2] = invVP * glm::vec4{ 1.f, -1.f, -1.f, 1.f};
	// corners[3] = invVP * glm::vec4{-1.f, -1.f, -1.f, 1.f};
	// corners[4] = invVP * glm::vec4{ 1.f,  1.f,  1.f, 1.f};
	// corners[5] = invVP * glm::vec4{-1.f,  1.f,  1.f, 1.f};
	// corners[6] = invVP * glm::vec4{ 1.f, -1.f,  1.f, 1.f};
	// corners[7] = invVP * glm::vec4{-1.f, -1.f,  1.f, 1.f};

	// corners[0] /= corners[0].w;
	// corners[1] /= corners[1].w;
	// corners[2] /= corners[2].w;
	// corners[3] /= corners[3].w;
	// corners[4] /= corners[4].w;
	// corners[5] /= corners[5].w;
	// corners[6] /= corners[6].w;
	// corners[7] /= corners[7].w;

	// cout << "corners[0]=" << glm::to_string(corners[0]) << endl;
	// cout << "corners[1]=" << glm::to_string(corners[1]) << endl;
	// cout << "corners[2]=" << glm::to_string(corners[2]) << endl;
	// cout << "corners[3]=" << glm::to_string(corners[3]) << endl;
	// cout << "corners[4]=" << glm::to_string(corners[4]) << endl;
	// cout << "corners[5]=" << glm::to_string(corners[5]) << endl;
	// cout << "corners[6]=" << glm::to_string(corners[6]) << endl;
	// cout << "corners[7]=" << glm::to_string(corners[7]) << endl;

	// cout << "zMinMax=" << glm::to_string(zMinMax) << endl;
	// cout << "lightDirection=" << glm::to_string(lightDirection) << endl;

	for (size_t i = 0; i < splitCount; i++)
	{
		// cout << "split[" << i << "]:" << splitNear[i] << "-" << splitFar[i] << endl;
		const auto splitP = glm::perspective(fovy, aspect, splitNear[i], splitFar[i]);
		const auto splitInvVP = glm::inverse(splitP * view.matrix);

		// corners[0] = splitInvVP * glm::vec4{ 1.f,  1.f, -1.f, 1.f};
		// corners[1] = splitInvVP * glm::vec4{-1.f,  1.f, -1.f, 1.f};
		// corners[2] = splitInvVP * glm::vec4{ 1.f, -1.f, -1.f, 1.f};
		// corners[3] = splitInvVP * glm::vec4{-1.f, -1.f, -1.f, 1.f};
		// corners[4] = splitInvVP * glm::vec4{ 1.f,  1.f,  1.f, 1.f};
		// corners[5] = splitInvVP * glm::vec4{-1.f,  1.f,  1.f, 1.f};
		// corners[6] = splitInvVP * glm::vec4{ 1.f, -1.f,  1.f, 1.f};
		// corners[7] = splitInvVP * glm::vec4{-1.f, -1.f,  1.f, 1.f};

		// corners[0] /= corners[0].w;
		// corners[1] /= corners[1].w;
		// corners[2] /= corners[2].w;
		// corners[3] /= corners[3].w;
		// corners[4] /= corners[4].w;
		// corners[5] /= corners[5].w;
		// corners[6] /= corners[6].w;
		// corners[7] /= corners[7].w;

		// cout << "corners[0]=" << glm::to_string(corners[0]) << endl;
		// cout << "corners[1]=" << glm::to_string(corners[1]) << endl;
		// cout << "corners[2]=" << glm::to_string(corners[2]) << endl;
		// cout << "corners[3]=" << glm::to_string(corners[3]) << endl;
		// cout << "corners[4]=" << glm::to_string(corners[4]) << endl;
		// cout << "corners[5]=" << glm::to_string(corners[5]) << endl;
		// cout << "corners[6]=" << glm::to_string(corners[6]) << endl;
		// cout << "corners[7]=" << glm::to_string(corners[7]) << endl;

		// const auto box = computeBox(splitProjection);

		// Ps[i] = glm::ortho<float>(splitAABB.min.x, splitAABB.max.x, splitAABB.min.y, splitAABB.max.y, -splitAABB.max.z, -splitAABB.min.z);

		// cout << "  max=" << glm::to_string(splitAABB.max) << endl;
		// cout << "  min=" << glm::to_string(splitAABB.min) << endl;

		const phs::Sphere splitSphere{splitP};
		glm::vec3 center = splitSphere.position;
		float radius = splitSphere.radius;

		glm::vec3 viewCenter{view.invMatrix * glm::vec4{center, 1.f}};
		float minViewCenterDistance = glm::dot(lightDirection, viewCenter);
		float zNearCorrection = min((minViewCenterDistance - radius) - zMinMax.x, 0.f);

		// cout << "center=" << glm::to_string(center) << endl;
		// cout << "viewCenter=" << glm::to_string(viewCenter) << endl;
		// cout << "radius=" << radius << endl;
		// cout << "minViewCenterDistance=" << minViewCenterDistance << endl;
		// cout << "minViewCenterDistance - radius=" << minViewCenterDistance - radius << endl;

		//
		// cout << "view center: " << glm::to_string(glm::vec3{view.invMatrix * glm::vec4{center, 1.f}}) << endl;
		// cout << "splitSphere: " << glm::to_string(splitSphere.position) << ", " << splitSphere.radius << endl;

		// cout << "  sphere=" << glm::to_string(center) << ", r=" << radius << endl;


		Ps[i] = glm::ortho(
			center.x - radius, center.x + radius,
			center.y - radius, center.y + radius,
			-center.z - radius + zNearCorrection, -center.z + radius
		);

		if (useViewCenter)
		{
			center = glm::vec3{view.invMatrix * glm::vec4{center, 1.f}};
		}

		Vs[i] = glm::lookAt(viewCenter, viewCenter - light.direction, glm::vec3{0.f, 1.f, 0.f});;

		// glm::vec2 q{1.f / (float) fbShadows[i].width, 1.f / (float) fbShadows[i].height};

		// auto rounded = glm::round(projCenter / q) * q;
		// auto offset = glm::vec3(projCenter.x - rounded.x, projCenter.y - rounded.y, 0.f);
		//
		// projection = glm::translate(projection, -offset);

		// _shadowProjections[i] = cameraViewProjection;

		cascades[i] = -splitFar[i];

		// cout << "  n - f=[" << splitNear[i] << " - " << splitFar[i] << "]" << endl;
	}

	//

	/*
	glm::vec4 frustumCorners[8];
	buildCorners(projection.matrix * view.matrix, frustumCorners);

	glm::vec3 cameraPosition = ecs::get<Position>(cameraId).position;

	glm::mat4 V = glm::lookAt(cameraPosition, cameraPosition - light.direction, glm::vec3{0.f, 1.f, 0.f});

	glm::vec4 splitCorners[] = {
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

	splitCorners[RTF] = splitCorners[RTN];
	splitCorners[LTF] = splitCorners[LTN];
	splitCorners[RBF] = splitCorners[RBN];
	splitCorners[LBF] = splitCorners[LBN];

	for (size_t i = 0; i < splits.size(); i++)
	{
		splitCorners[RTN] = splitCorners[RTF];
		splitCorners[LTN] = splitCorners[LTF];
		splitCorners[RBN] = splitCorners[RBF];
		splitCorners[LBN] = splitCorners[LBF];
		splitCorners[RTF] = glm::lerp(frustumCorners[RTN], frustumCorners[RTF], splits[i]);
		splitCorners[LTF] = glm::lerp(frustumCorners[LTN], frustumCorners[LTF], splits[i]);
		splitCorners[RBF] = glm::lerp(frustumCorners[RBN], frustumCorners[RBF], splits[i]);
		splitCorners[LBF] = glm::lerp(frustumCorners[LBN], frustumCorners[LBF], splits[i]);

		cascades[i] = -glm::lerp(projection.zNear, projection.zFar, splits[i]);

		Ps[i] = buildShadowPMatrix(splitCorners, V, zMax);
		Vs[i] = stabilizeVMatrix(V, Ps[i]);
	}
	*/
}

void CSM::render()
{
	profRender.start();

	RN_SCOPE_DISABLE(GL_BLEND);
	RN_SCOPE_DISABLE(GL_STENCIL_TEST);
	RN_SCOPE_DISABLE(GL_CULL_FACE);

	progDepth.use();

	progDepth.uniform("writeColor", false);

	// RN_CHECK(glCullFace(GL_FRONT));

	for (size_t i = 0; i < Ps.size(); i++)
	{
		const auto &P = Ps[i];
		const auto &V = Vs[i];
		progDepth.uniform("P", P);
		progDepth.uniform("V", V);

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

	// RN_CHECK(glCullFace(GL_BACK));

	progDepth.forgo();

	profRender.stop();

	// profBlur.start();

	// RN_SCOPE_DISABLE(GL_DEPTH_TEST);

	// for (size_t i = 0; i < fbShadows.size() - 1; i++)
	// {
	// 	auto &fbShadow = fbShadows[i];

	// 	{
	// 		RN_FB_BIND(fbBlurBuffer);

	// 		fbBlurBuffer.clear(rn::BUFFER_COLOR);

	// 		progBlurH.use();
	// 		// progBlurH.var("texSource", fbShadow.color(0)->bind(0));
	// 		progBlurH.var("texSource", texCascades->bind(0));
	// 		progBlurH.var("layer", static_cast<GLint>(i));

	// 		rn::Mesh::quad.render();
	// 		progBlurH.forgo();
	// 	}

	// 	{
	// 		RN_FB_BIND(fbShadow);

	// 		fbShadow.clear(rn::BUFFER_COLOR);

	// 		progBlurV.use();
	// 		progBlurV.var("texSource", fbBlurBuffer.color(0)->bind(0));
	// 		progBlurV.var("layer", static_cast<GLint>(i));

	// 		rn::Mesh::quad.render();
	// 		progBlurV.forgo();
	// 	}
	// }

	// profBlur.stop();
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

glm::vec2 CSM::findSceneZMinMax(glm::vec3 lightDirection)
{
	glm::vec2 zMinMax{numeric_limits<float>::max(), -numeric_limits<float>::max()};

	for (auto &entity : ecs::findWith<Transform, Mesh, BoundingObject, Occluder>())
	{
		auto &boundingObject = ecs::get<BoundingObject>(entity);

		float dist = glm::dot(lightDirection, boundingObject.sphere.position) + boundingObject.sphere.radius;

		zMinMax.x = min(zMinMax.x, dist);
		zMinMax.y = max(zMinMax.y, dist);
	}

	return zMinMax;
}

glm::mat4 CSM::buildShadowPMatrix(const glm::vec4 (&corners)[8], const glm::mat4 &V, float zMax)
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

	float left = shadowFrustumCorners[0].x;
	float right = shadowFrustumCorners[0].x;
	float bottom = shadowFrustumCorners[0].y;
	float top = shadowFrustumCorners[0].y;
	float zNear = shadowFrustumCorners[0].z;
	float zFar = shadowFrustumCorners[0].z;

	for (size_t i = 1; i < 8; i++)
	{
		left = min(left, shadowFrustumCorners[i].x);
		right = max(right, shadowFrustumCorners[i].x);
		bottom = min(bottom, shadowFrustumCorners[i].y);
		top = max(top, shadowFrustumCorners[i].y);
		zNear = max(zNear, shadowFrustumCorners[i].z);
		zFar = min(zFar, shadowFrustumCorners[i].z);
	}

	zNear = max(zNear, zMax);

	return glm::ortho(left, right, bottom, top, -zNear, -zFar);
}

glm::mat4 CSM::stabilizeVMatrix(const glm::mat4 &V, const glm::mat4 &P)
{
	// reduce shimmering
	// glm::vec2 texelSize{(right - left) / texDepths->width, (top - bottom) / texDepths->height};

	// left -= glm::mod(left, texelSize.x);
	// right += texelSize.x - glm::mod(right, texelSize.x);

	// bottom -= glm::mod(bottom, texelSize.y);
	// top += texelSize.y - glm::mod(top, texelSize.y);

	return V;
}

pair<glm::vec3, glm::vec3> CSM::computeBox(const glm::mat4 &splitProjection)
{
	glm::mat4 invP = glm::inverse(splitProjection);

	glm::vec4 corners[] = {
		invP * glm::vec4{-1.f,  1.f, -1.f, 1.f},
		invP * glm::vec4{ 1.f,  1.f, -1.f, 1.f},
		invP * glm::vec4{ 1.f, -1.f, -1.f, 1.f},
		invP * glm::vec4{-1.f, -1.f, -1.f, 1.f},
		invP * glm::vec4{-1.f,  1.f,  1.f, 1.f},
		invP * glm::vec4{ 1.f,  1.f,  1.f, 1.f},
		invP * glm::vec4{ 1.f, -1.f,  1.f, 1.f},
		invP * glm::vec4{-1.f, -1.f,  1.f, 1.f}
	};

	corners[0] = corners[0] / corners[0].w;
	corners[1] = corners[1] / corners[1].w;
	corners[2] = corners[2] / corners[2].w;
	corners[3] = corners[3] / corners[3].w;
	corners[4] = corners[4] / corners[4].w;
	corners[5] = corners[5] / corners[5].w;
	corners[6] = corners[6] / corners[6].w;
	corners[7] = corners[7] / corners[7].w;

	glm::vec3 bottomLeft{numeric_limits<float>::max()};
	glm::vec3 topRight{-std::numeric_limits<float>::max()};

	for (const auto &c : corners)
	{
		bottomLeft.x = min(bottomLeft.x, c.x);
		bottomLeft.y = min(bottomLeft.y, c.y);
		bottomLeft.z = min(bottomLeft.z, c.z);
		topRight.x = max(topRight.x, c.x);
		topRight.y = max(topRight.y, c.y);
		topRight.z = max(topRight.z, c.z);
	}

	return { bottomLeft, topRight };
}

} // fx