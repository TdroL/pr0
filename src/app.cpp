#include "app.hpp"

#include <iostream>

#include "core/gl.hpp"
#include "core/sys.hpp"
#include "core/sys/key.hpp"
#include "core/sys/window.hpp"
#include "core/src/sbm.hpp"

namespace key = sys::key;
namespace win = sys::window;

using namespace std;

void App::init()
{
	/* Init scene objects */
	clog << "Init scene objects:" << endl;

	material.shininess = 0.125f;

	light.ambient = glm::vec4{0.f, 0.3125f * 0.125f, 1.f * 0.125f, 1.f};
	light.position = glm::vec4{0.f, 1.f, 0.f, 1.f};
	light.linearAttenuation = 1.0 / (3.0 * 3.0); // r_l = 3.0; // linear attenuation; distance at which half of the light intensity is lost
	light.quadraticAttenuation = 1.0 / (6.0 * 6.0); // r_q = 6.0; // quadriatic attenuation; distance at which three-quarters of the light intensity is lost

	try
	{
		prog.load("lighting/specular.frag", "PN.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	try
	{
		simple.load("color.frag", "PN.vert");
	}
	catch (const string &e)
	{
		cerr << "Warning: " << e << endl;
	}

	simple.uniform("color", glm::vec3{1.f});

	glm::mat4 projectionMatrix = glm::perspective(45.f, static_cast<float>(win::width) / static_cast<float>(win::height), 1.f/128.0f, 1000.0f);
	prog.uniform("P", projectionMatrix);
	simple.uniform("P", projectionMatrix);


	suzanne.load(src::sbm::mesh("suzanne.sbm"));
	venus.load(src::sbm::mesh("venus.sbm"));
	sphere.load(src::sbm::mesh("sphere.sbm"));
	plane.load(src::sbm::mesh("plane.sbm"));
	// stargate.load(src::sbm::mesh("stargate.sbm"));

	fbo.create(3);
}

void App::update()
{
	glm::vec3 position{0.0, 0.0, 0.0};
	glm::vec3 rotation{0.0, 0.0, 0.0};

	position.z = (key::pressed('s') - key::pressed('w'));
	position.x = (key::pressed('d') - key::pressed('a'));
	position.y = (key::pressed(KEY_SPACE) - key::pressed(KEY_CTRL));

	if (position.x != 0.f && position.y != 0.f && position.z != 0.f)
	{
		position = glm::normalize(position);

		float speed = 10.f;
		position *= speed * sys::dt;
	}

	rotation.y = (key::pressed(KEY_LEFT) - key::pressed(KEY_RIGHT));
	rotation.x = (key::pressed(KEY_UP) - key::pressed(KEY_DOWN));

	camera.apply(rotation, position);

	lightPositionDelta = glm::vec4{5.0 * sys::dt};

	lightPositionDelta.z *= (sys::key::pressed(KEY_KP_5) - sys::key::pressed(KEY_KP_8));
	lightPositionDelta.x *= (sys::key::pressed(KEY_KP_6) - sys::key::pressed(KEY_KP_4));
	lightPositionDelta.y *= (sys::key::pressed(KEY_KP_ADD) - sys::key::pressed(KEY_KP_ENTER));
	lightPositionDelta.w *= 0.f;
}

void App::render()
{
	GL_CHECK(glClearColor(0.0f, 0.3125f, 1.f, 1.f));
	GL_CHECK(glClearDepth(1.f));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	{
		GL_FBO_USE(fbo);
		fbo.clear();

		glm::mat4 &V = camera.viewMatrix;

		prog.use();
		prog.var("V", V);

		prog.var("matAmbient", material.ambient);
		prog.var("matDiffuse", material.diffuse);
		prog.var("matSpecular", material.specular);
		prog.var("matEmission", material.emission);
		prog.var("matShininess", material.shininess);

		light.position += lightPositionDelta;

		prog.var("lightAmbient", light.ambient);
		prog.var("lightDiffuse", light.diffuse);
		prog.var("lightSpecular", light.specular);
		prog.var("lightPosition", light.position);
		// prog.var("lightSpotDirection", light.spotDirection);
		// prog.var("lightSpotExponent", light.spotExponent);
		// prog.var("lightSpotCutoff", light.spotCutoff);
		// prog.var("lightConstantAttenuation", light.constantAttenuation);
		prog.var("lightLinearAttenuation", light.linearAttenuation);
		prog.var("lightQuadraticAttenuation", light.quadraticAttenuation);

		{
			glm::mat4 M = glm::rotate(glm::translate(glm::mat4{1.f}, glm::vec3{2.f, 0.f, 0.f}), static_cast<float>(90.0 * sys::time()), glm::vec3{0.f, 1.f, 0.f});
			glm::mat3 N{V * M};
			prog.var("M", M);
			prog.var("N", N);
			suzanne.render();
		}

		{
			glm::mat4 M = glm::rotate(glm::translate(glm::mat4{1.f}, glm::vec3{-2.f, 0.f, 0.f}), static_cast<float>(0.0 * -90.0 * sys::time()), glm::vec3{0.f, 1.f, 0.f});
			glm::mat3 N{V * M};
			prog.var("M", M);
			prog.var("N", N);
			venus.render();
		}

		// {
		// 	glm::mat4 M = glm::scale(glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 1.125f, -3.f}), glm::vec3{0.25f});
		// 	glm::mat3 N{V * M};
		// 	prog.var("M", M);
		// 	prog.var("N", N);
		// 	stargate.render();
		// }

		{
			glm::mat4 M = glm::scale(glm::translate(glm::mat4{1.f}, glm::vec3{0.f, -1.5f, 0.f}), glm::vec3{100.f});
			glm::mat3 N{V * M};
			prog.var("M", M);
			prog.var("N", N);
			plane.render();
		}

		simple.use();
		simple.var("V", V);
		simple.var("M", glm::scale(glm::translate(glm::mat4{1.f}, glm::vec3{light.position}), glm::vec3{0.25f}));
		sphere.render();
	}

	fbo.render();
}