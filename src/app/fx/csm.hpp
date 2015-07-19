#ifndef APP_FX_CSM_HPP
#define APP_FX_CSM_HPP

#include <app/comp/projection.hpp>

#include <core/rn/fb.hpp>
#include <core/rn/prof.hpp>
#include <core/rn/program.hpp>
#include <core/rn/tex2darray.hpp>
#include <core/ecs/entity.hpp>

#include <vector>
#include <utility>

#include <glm/glm.hpp>

namespace fx
{

class CSM
{
public:
	bool useSmartSplitting = true;

	std::vector<float> splits{ 0.025f, 0.075f, 0.25f, 1.f };
	std::vector<float> cascades{};

	int shadowResolution = 1024;

	std::shared_ptr<rn::Tex2DArray> texDepths{};
	// std::shared_ptr<rn::Tex2DArray> texCascades{};
	std::vector<rn::FB> fbShadows{};
	rn::FB fbBlurBuffer{};

	std::vector<glm::mat4> Ps{};
	std::vector<glm::mat4> Vs{};

	rn::Program progDepth{};
	// rn::Program progBlurH{};
	// rn::Program progBlurV{};

	rn::Prof profRender{"fx::CSM::profRender"};
	rn::Prof profBlur{"fx::CSM::profBlur"};

	ecs::Entity cameraId{};

	void init();

	void setup(const ecs::Entity &lightId);
	void render();

	void buildCorners(const glm::mat4 &VP, glm::vec4 (&output)[8]);
	glm::mat4 buildShadowPMatrix(const glm::vec4 (&corners)[8], const glm::mat4 &V, float zMax);
	glm::mat4 stabilizeVMatrix(const glm::mat4 &V, const glm::mat4 &P);
	glm::vec2 findSceneZMinMax(glm::vec3 lightDirection);
	std::pair<glm::vec3, glm::vec3> computeBox(const glm::mat4 &splitProjection);
};

} // fx

#endif