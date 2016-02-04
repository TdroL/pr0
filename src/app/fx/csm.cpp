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
#include <core/rn.hpp>
#include <core/rn/math.hpp>
#include <core/rn/mesh.hpp>
#include <core/util/count.hpp>

#include <minball/minball.hpp>

#include <iostream>
#include <cmath>
#include <utility>
#include <limits>
#include <sstream>
#include <iomanip>

namespace fx
{

using namespace std;
using namespace comp;

constexpr size_t CSM::maxCascades;
constexpr size_t CSM::textureResolution;

void CSM::init()
{
	fbShadows.resize(splits);

	Ps.resize(splits);
	Vs.resize(splits);
	radiuses2.resize(splits);
	shadowBiasedVPs.resize(splits);
	centers.resize(splits);

	texDepths = make_shared<rn::Tex2DArray>("fx::CSM::texDepths");
	texDepths->width = textureResolution;
	texDepths->height = textureResolution;
	texDepths->size = splits;
	texDepths->wrapS = rn::WRAP_BORDER;
	texDepths->wrapT = rn::WRAP_BORDER;
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

	{
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
	}

	float radiusPadding = static_cast<float>(textureResolution) / static_cast<float>(textureResolution - 1);
	float radiusBias = 1.f;
	glm::mat4 biasMatrix{
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	};

	for (size_t j = splits; j > 0; j--)
	{
		size_t i = j - 1;

		phs::Sphere splitSphere{glm::perspective(fovy, aspect, splitNear[i], splitFar[i])};

		glm::vec3 center = glm::vec3{view.invMatrix * glm::vec4{splitSphere.position, 1.f}};
		float radius = splitSphere.radius * radiusPadding + radiusBias;

		float zCenter = glm::dot(lightDirection, center) + radius;
		float zNearCorrection = max(0.f, zMax - zCenter);

		glm::mat4 P = rn::math::orthoLHMatrix(
			-radius, +radius,
			-radius, +radius,
			-radius - zNearCorrection, +radius
		);

		// stabilize cascade center
		float qStep = (2.f * radius) / static_cast<float>(textureResolution);

		glm::mat4 V = glm::lookAt(center, center - lightDirection, glm::vec3{0.f, 1.f, 0.f});
		V[3] -= glm::vec4{glm::mod(V[3].xyz(), qStep), 0.f};

		Ps[i] = P;
		Vs[i] = V;
		shadowBiasedVPs[i] = biasMatrix * P * V * view.invMatrix;
		centers[i] = glm::vec3{view.matrix  * glm::vec4{center, 1.f}};
		radiuses2[i] = (radius - qStep) * (radius - qStep);
	}
}

} // fx