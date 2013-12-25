#ifndef APP_HPP
#define APP_HPP

#include "core/cam/basic.hpp"
#include "core/gl.hpp"
#include "core/gl/fbo.hpp"

#include "core/gl/mesh.hpp"
#include "core/gl/program.hpp"

class App
{
public:

	struct Material
	{
		glm::vec4 ambient{0.2f, 0.2f, 0.2f, 1.f};
		glm::vec4 diffuse{0.8f, 0.8f, 0.8f, 1.f};
		glm::vec4 specular{0.f, 0.f, 0.f, 1.f};
		glm::vec4 emission{0.f, 0.f, 0.f, 1.f};
		GLfloat shininess = 0.f;
		GLfloat padding[3];
	};

	struct Light
	{
		glm::vec4 ambient{0.f, 0.f, 0.f, 1.f};
		glm::vec4 diffuse{1.f, 1.f, 1.f, 1.f};
		glm::vec4 specular{1.f, 1.f, 1.f, 1.f};
		glm::vec4 position{0.f, 0.f, 1.f, 0.f};
		// glm::vec3 spotDirection{0.f, 0.f, -1.f};
		// GLfloat spotExponent = 0.f;
		// GLfloat spotCutoff = 180.f;
		// GLfloat constantAttenuation = 1.f;
		GLfloat linearAttenuation = 0.f;
		GLfloat quadraticAttenuation = 0.f;
		GLfloat padding[2];
	};

	glm::vec4 lightPositionDelta{0.f};

	/*
	struct Material
	{
		glm::vec4 emission{1.f, 1.f, 1.f, 1.f};
		glm::vec4 specular{1.f, 1.f, 1.f, 1.f};
		GLfloat shininess = 0.125f;
		GLfloat padding[3];
	};

	struct Light
	{
		glm::vec4 diffuse{0.5f, 0.5f, 0.5f, 1.f};
		glm::vec4 ambient{0.f, 0.3125f * 0.125f, 1.f * 0.125f, 1.f};
		glm::vec3 position{0.f, 1.f, 0.f};
		GLfloat attenuationL = 1.0 / (3.0 * 3.0); // r_l = 3.0; // linear attenuation; distance at which half of the light intensity is lost
		GLfloat attenuationQ = 1.0 / (6.0 * 6.0); // r_q = 10.0; // quadriatic attenuation; distance at which three-quarters of the light intensity is lost
		GLfloat padding[3];
	};
	*/

	Material material{};
	Light light{};

	cam::Basic camera{glm::vec3{0.f, 0.f, 8.f}};

	gl::Program prog{};
	gl::Program simple{};

	gl::Mesh suzanne{"suzanne"};
	gl::Mesh venus{"venus"};
	gl::Mesh sphere{"sphere"};
	gl::Mesh plane{"plane"};
	gl::Mesh stargate{"stargate"};

	gl::FBO fbo{"screen"};

	App() {};
	~App() {};

	void init();

	void update();

	void render();
};

#endif