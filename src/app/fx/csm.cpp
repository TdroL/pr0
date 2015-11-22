#include <pch.hpp>

#include "csm.hpp"

#include <app/comp/boundingvolume.hpp>
#include <app/comp/mesh.hpp>
#include <app/comp/occluder.hpp>
#include <app/comp/transform.hpp>
#include <app/proc/frustumprocess.hpp>
#include <app/proc/meshrenderer.hpp>

#include <core/phs/aabb.hpp>
#include <core/phs/frustum.hpp>
#include <core/rn/mesh.hpp>
#include <core/util/count.hpp>

#include <minball/minball.hpp>

#include <iostream>
#include <cmath>
#include <utility>
#include <limits>
#include <sstream>
#include <iomanip>

#include <glm/gtx/compatibility.hpp>

namespace fx
{

using namespace std;
using namespace comp;

void CSM::init()
{
	fbShadows.resize(splits);

	radiuses2.resize(splits);
	Ps.resize(splits);
	Vs.resize(splits);
	shadowBiasedVPs.resize(splits);
	centers.resize(splits);

	texDepths = make_shared<rn::Tex2DArray>("fx::CSM::texDepths");
	texDepths->width = textureResolution;
	texDepths->height = textureResolution;
	texDepths->size = splits;
	texDepths->minFilter = rn::MinFilter::MIN_NEAREST;
	texDepths->magFilter = rn::MagFilter::MAG_NEAREST;
	texDepths->internalFormat = rn::format::D32F.layout;
	texDepths->reload();

	for (size_t i = 0; i < fbShadows.size(); i++)
	{
		auto &fb = fbShadows[i];
		fb.fbName = "fx::CSM::fbShadows[" + to_string(i) + "]";
		fb.clearDepthValue = 1.f;

		fb.attachDepth(texDepths, i);
		fb.reload();
	}

	try
	{
		progDepth.load("fx/csm/depth.frag", "fx/csm/depth.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	// profRender.init();
}

void CSM::calculateMatrices(const DirectionalLight &light, const Projection &projection, const View &view, float zMax)
{
	const float zNear = projection.zNear;
	const float zFar = min(projection.zFar, maxShadowDistance);
	const float fovy = projection.fovy;
	const float aspect = projection.aspect;

	const auto lightDirection = glm::normalize(light.direction);

	vector<float> splitFar(splits, zFar);
	vector<float> splitNear(splits, zNear);

	float lambda = 0.8f;
	float j = 1.f;

	for (size_t i = 0; i < splits; i++)
	{
		splitFar[i] = glm::mix(
			zNear + (j / (float) splits) * (zFar - zNear),
			zNear * pow(zFar / zNear, j / (float) splits),
			lambda
		);
		j += 1.f;

		if (i + 1 < splits)
		{
			splitNear[i + 1] = splitFar[i];
		}
	}

	float radiusPadding = static_cast<float>(textureResolution) / static_cast<float>(textureResolution - 1);
	float radiusBias = 1.f;
	glm::mat4 xyBias{
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	};

	for (size_t j = splits; j > 0; j--)
	{
		size_t i = j - 1;

		glm::mat4 splitP = glm::perspective(fovy, aspect, splitNear[i], splitFar[i]);
		phs::Sphere splitSphere{splitP};

		glm::vec3 center = glm::vec3{view.invMatrix * glm::vec4{splitSphere.position, 1.f}};
		float radius = splitSphere.radius * radiusPadding + radiusBias;

		float c_l_r = glm::dot(lightDirection, center) + radius;
		float zNearCorrection = max(c_l_r, zMax) - c_l_r;

		Ps[i] = glm::ortho(
			-radius, +radius,
			-radius, +radius,
			-radius - zNearCorrection, +radius
		);

		// stabilize cascade center
		float qStep = (2.f * radius) / static_cast<float>(textureResolution);

		Vs[i] = glm::lookAt(center, center - lightDirection, glm::vec3{0.f, 1.f, 0.f});
		Vs[i][3].x -= glm::mod(Vs[i][3].x, qStep);
		Vs[i][3].y -= glm::mod(Vs[i][3].y, qStep);
		Vs[i][3].z -= glm::mod(Vs[i][3].z, qStep);

		shadowBiasedVPs[i] = xyBias * Ps[i] * Vs[i] * view.invMatrix;

		radiuses2[i] = (radius - qStep) * (radius - qStep);
		centers[i] = glm::vec3{view.matrix  * glm::vec4{center, 1.f}};
	}
}

// void CSM::renderCascades()
// {
// 	profRender.start();

// 	RN_CHECK(glDepthFunc(GL_LEQUAL));

// 	progDepth.use();

// 	progDepth.uniform("writeColor", false);

// 	RN_CHECK(glCullFace(GL_FRONT));

// 	for (size_t i = 0; i < Ps.size(); i++)
// 	{
// 		const auto &P = Ps[i];
// 		const auto &V = Vs[i];
// 		const auto VP = P * V;
// 		progDepth.uniform("P", P);
// 		progDepth.uniform("V", V);

// 		auto &fbShadow = fbShadows[i];

// 		RN_FB_BIND(fbShadow);

// 		fbShadow.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH);

// 		const phs::Frustum boundingFrustum{VP};

// 		for (auto &entity : ecs::findWith<Transform, Mesh, Occluder>())
// 		{
// 			if (ecs::has<BoundingVolume>(entity) && ! proc::FrustumProcess::isVisible(entity, boundingFrustum))
// 			{
// 				continue;
// 			}

// 			proc::MeshRenderer::render(entity, progDepth);
// 		}
// 	}

// 	RN_CHECK(glCullFace(GL_BACK));

// 	progDepth.forgo();

// 	RN_CHECK(glDepthFunc(rn::Default::depthFunc /*GL_GEQUAL*/));

// 	profRender.stop();
// }

// glm::vec2 CSM::findSceneZMinMax(glm::vec3 lightDirection)
// {
// 	//# cout << "findSceneZMinMax" << endl;
// 	//# cout << "  lightDirection=" << glm::to_string(lightDirection) << endl;

// 	float zMax = -numeric_limits<float>::max();

// 	for (auto &entity : ecs::findWith<Transform, Mesh, BoundingVolume, Occluder>())
// 	{
// 		auto &boundingVolume = ecs::get<BoundingVolume>(entity);
// 		float dist = glm::dot(lightDirection, boundingVolume.sphere.position) + boundingVolume.sphere.radius;

// 		zMax = max(zMax, dist);
// 	}

// 	return zMax;
// }

} // fx