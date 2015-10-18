#include <pch.hpp>

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

#include <minball/minball.hpp>

#include <iostream>
#include <cmath>
#include <utility>
#include <limits>
#include <sstream>
#include <iomanip>

// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
// #include <glm/gtx/string_cast.hpp>

namespace fx
{

using namespace std;
using namespace comp;

// enum CORNERS {
// 	RTN = 0,
// 	LTN = 1,
// 	RBN = 2,
// 	LBN = 3,
// 	RTF = 4,
// 	LTF = 5,
// 	RBF = 6,
// 	LBF = 7
// };

void CSM::init()
{
	cascades.resize(splits);
	radiuses.resize(splits);
	centers.resize(splits);
	fbShadows.resize(splits);
	Ps.resize(splits);
	Vs.resize(splits);

	radiuses2.resize(splits);
	VPs.resize(splits);
	centersV.resize(splits);

	texDepths = make_shared<rn::Tex2DArray>("fx::CSM::texDepths");
	texDepths->width = textureResolution;
	texDepths->height = textureResolution;
	texDepths->size = splits;
	texDepths->minFilter = rn::MinFilter::MIN_NEAREST;
	texDepths->magFilter = rn::MagFilter::MAG_NEAREST;
	// texDepths->compareFunc = rn::CompareFunc::COMPARE_GEQUAL;
	texDepths->internalFormat = rn::format::D32F.layout;
	texDepths->reload();

	texColors = make_shared<rn::Tex2DArray>("fx::CSM::texColors");
	texColors->width = textureResolution;
	texColors->height = textureResolution;
	texColors->size = splits;
	texColors->internalFormat = rn::format::RGBA32F.layout;
	texColors->reload();

	for (size_t i = 0; i < fbShadows.size(); i++)
	{
		auto &fb = fbShadows[i];

		fb.fbName = "fx::CSM::fbShadows[" + to_string(i) + "]";

		fb.attachColor(0, texColors, i);
		fb.attachDepth(texDepths, i);

		fb.clearColorValue = glm::vec4{1.f};
		fb.clearDepthValue = 1.f;
		fb.reload();
	}

	// auto texBlurBuffer = make_shared<rn::Tex2D>("fx::CSM::texBlurBuffer");

	// texBlurBuffer->width = texColors->width;
	// texBlurBuffer->height = texColors->height;
	// texBlurBuffer->internalFormat = texColors->internalFormat;
	// texBlurBuffer->reload();

	// fbBlurBuffer.fbName = "fx::CSM::fbBlurBuffer";
	// fbBlurBuffer.attachColor(0, texBlurBuffer);
	// fbBlurBuffer.reload();

	try
	{
		progDepth.load("fx/csm/depth.frag", "fx/csm/depth.vert");
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

void CSM::calculateMatrices(const ecs::Entity &cameraId, const ecs::Entity &lightId)
{
	const auto &light = ecs::get<DirectionalLight>(lightId);

	const auto &projection = ecs::get<Projection>(cameraId);
	const auto &view = ecs::get<View>(cameraId);
	// const auto &cameraPosition = ecs::get<Position>(cameraId).position;

	const float zNear = projection.zNear;
	const float zFar = min(projection.zFar, maxShadowDistance);
	const float fovy = projection.fovy;
	const float aspect = projection.aspect;

	const auto lightDirection = glm::normalize(light.direction);
	glm::vec2 zMinMax = findSceneZMinMax(lightDirection);

	//# cout << "  position=" << glm::to_string(cameraPosition) << endl;

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
	glm::mat4 V = glm::lookAt(glm::vec3{0.f}, -lightDirection, glm::vec3{0.f, 1.f, 0.f});
	glm::mat4 invV = glm::inverse(V);

	// cout << "l=" << glm::to_string(light.direction) << endl;
	// cout << "V * l=" << glm::to_string(V * glm::vec4{light.direction, 1.f}) << endl;
	// cout << "V' * l=" << glm::to_string(invV * (V * glm::vec4{light.direction, 1.f})) << endl;

	debugLog = "";

	for (size_t j = splits; j > 0; j--)
	{
		size_t i = j - 1;

		glm::mat4 splitP = glm::perspective(fovy, aspect, splitNear[i], splitFar[i]);
		// glm::mat4 invProj = glm::inverse(splitP);
		phs::Sphere splitSphere{splitP};

		// glm::mat4 invProj = invV * glm::inverse(splitP * V);

		//# cout << "  [" << i << "] camPosition=" << glm::to_string(ecs::get<Position>(cameraId).position) << endl;
		//# cout << "  [" << i << "] splitSphere.position=" << glm::to_string(glm::vec3{splitSphere.position}) << endl;
		//# cout << "  [" << i << "] splitSphere.radius=" << splitSphere.radius << endl;
		glm::vec3 center = glm::vec3{view.invMatrix * glm::vec4{splitSphere.position, 1.f}};
		//# cout << "  [" << i << "] center=" << glm::to_string(center) << endl;
		float radius = splitSphere.radius * radiusPadding + radiusBias;

		// correct near plane to include objects behind cascade bounding sphere
		float c_l = glm::dot(lightDirection, center);
		float c_l_r = c_l + radius;
		// float zNearCorrection = max(c_l + radius, zMinMax.y) - zMinMax.y;
		float zNearCorrection = max(c_l_r, zMinMax.y) - c_l_r;

		//# cout << "  [" << i << "] zNearCorrection=" << zNearCorrection << endl;
		//# cout << "  [" << i << "] center=" << glm::to_string(center) << endl;
		//# cout << "  [" << i << "] c_l=" << c_l << endl;
		//# cout << "  [" << i << "] radius=" << radius << endl;

		// stabilize cascade center
		float qStep = (2.f * radius) / static_cast<float>(textureResolution);
		glm::vec4 centerV{V * glm::vec4{center, 1.f}};

		glm::vec4 centerVQuant = centerV;
		centerVQuant.x = floor(centerVQuant.x / qStep) * qStep;
		centerVQuant.y = floor(centerVQuant.y / qStep) * qStep;
		centerVQuant.z = floor(centerVQuant.z / qStep) * qStep;

		glm::vec3 centerQuant = glm::vec3{invV * centerVQuant};

		// cout << "i=" << i << endl;
		// cout << "manualCenter=" << glm::to_string(manualCenter) << endl;
		// cout << "manualRadius=" << manualRadius << endl;
		// cout << "position=" << glm::to_string(splitSphere.position) << endl;
		// cout << "radius=" << splitSphere.radius << endl;
		// cout << "center=" << glm::to_string(center) << endl;
		// cout << "centerV=" << glm::to_string(centerV) << endl;
		// cout << "centerInvV=" << glm::to_string(invV * glm::vec4{center, 1.f}) << endl;
		// cout << "centerVInvV=" << glm::to_string(invV * centerV) << endl;

		cascades[i] = splitFar[i];
		radiuses[i] = radius - qStep;
		centers[i] = glm::vec3{center};

		// cout << "qStep=" << qStep << endl;
		// cout << "x mod qStep=" << glm::mod(centerV.x, qStep) << endl;
		// cout << "y mod qStep=" << glm::mod(centerV.y, qStep) << endl;

		// centerV.x = floor(centerV.x / qStep) * qStep;
		// centerV.x -= glm::mod(centerV.x, qStep);
		// centerV.y = floor(centerV.y / qStep) * qStep;
		// centerV.y -= glm::mod(centerV.y, qStep);
		// centerV.z = floor(centerV.z / qStep) * qStep;
		// centerV.z -= glm::mod(centerV.z, qStep);

		// center = glm::vec3{invV * centerV};

		/**/
		Ps[i] = glm::ortho(
			-radius, +radius,
			-radius, +radius,
			-radius - zNearCorrection, +radius
		);
		/*/
		float right = radius;
		float left = -radius;
		float top = radius;
		float bottom = -radius;
		float zFar = radius;
		float zNear = -radius;

		auto &P = Ps[i] = glm::mat4{1.f};
		P[0][0] = 2.f / (right - left);
		P[1][1] = 2.f / (top - bottom);
		P[2][2] = 1.f / (zFar - zNear);
		P[3][0] = (left + right) / (left - right);
		P[3][1] = (bottom + top) / (bottom - top);
		P[3][2] = - zNear / (zFar - zNear);
		/**/

		Vs[i] = glm::lookAt(centerQuant, centerQuant - lightDirection, glm::vec3{0.f, 1.f, 0.f});
		// Vs[i] = glm::lookAt(center, center - light.direction, glm::vec3{0.f, 1.f, 0.f});

		//# cout << "  [" << i << "] splitNear=" << splitNear[i] << endl;
		//# cout << "  [" << i << "] splitFar=" << splitFar[i] << endl;
		//# cout << "  [" << i << "] centerQuant=" << glm::to_string(centerQuant) << endl;
		//# cout << "  [" << i << "] centerQuant - lightDirection=" << glm::to_string(centerQuant - lightDirection) << endl;

		VPs[i] = Ps[i] * Vs[i];
		radiuses2[i] = radiuses[i] * radiuses[i];
		centersV[i] = glm::vec3{view.matrix  * glm::vec4{centers[i], 1.f}};
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

	for (size_t i = 0; i < splits; i++)
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

void CSM::renderCascades()
{
	profRender.start();

	// RN_SCOPE_DISABLE(GL_BLEND);
	// RN_SCOPE_DISABLE(GL_STENCIL_TEST);
	RN_SCOPE_DISABLE(GL_CULL_FACE);

	RN_CHECK(glDepthFunc(GL_LEQUAL));

	progDepth.use();

	progDepth.uniform("writeColor", false);

	// RN_CHECK(glCullFace(GL_FRONT));

	for (size_t i = 0; i < Ps.size(); i++)
	{
		const auto &P = Ps[i];
		const auto &V = Vs[i];
		const auto VP = P * V;
		progDepth.uniform("P", P);
		progDepth.uniform("V", V);

		auto &fbShadow = fbShadows[i];

		RN_FB_BIND(fbShadow);

		fbShadow.clear(rn::BUFFER_COLOR | rn::BUFFER_DEPTH);

		const phs::Frustum boundingFrustum{VP};

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

	RN_CHECK(glDepthFunc(rn::Default::depthFunc /*GL_GEQUAL*/));

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
	// 		progBlurH.var("texSource", texColors->bind(0));
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

/*
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
*/

glm::vec2 CSM::findSceneZMinMax(glm::vec3 lightDirection)
{
	//# cout << "findSceneZMinMax" << endl;
	//# cout << "  lightDirection=" << glm::to_string(lightDirection) << endl;

	glm::vec2 zMinMax{numeric_limits<float>::max(), -numeric_limits<float>::max()};

	for (auto &entity : ecs::findWith<Transform, Mesh, BoundingObject, Occluder>())
	{
		auto &boundingObject = ecs::get<BoundingObject>(entity);

		float dist = glm::dot(lightDirection, boundingObject.sphere.position);

		zMinMax.x = min(zMinMax.x, dist - boundingObject.sphere.radius);
		zMinMax.y = max(zMinMax.y, dist + boundingObject.sphere.radius);
	}

	//# cout << "  zMinMax=" << glm::to_string(zMinMax) << endl;

	return zMinMax;
}

/*
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
*/

/*
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
*/

/*
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
*/

} // fx