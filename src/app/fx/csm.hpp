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

	size_t splits = 4;
	int shadowResolution = 1024*2;
	float maxShadowDistance = 256.f;

	std::vector<float> cascades{};
	std::vector<float> radiuses{};
	std::vector<glm::vec3> centers{};
	std::shared_ptr<rn::Tex2DArray> texDepths{};
	std::shared_ptr<rn::Tex2DArray> texColors{};
	std::vector<rn::FB> fbShadows{};
	// rn::FB fbBlurBuffer{};

	std::vector<glm::mat4> Ps{};
	std::vector<glm::mat4> Vs{};

	rn::Program progDepth{};
	// rn::Program progBlurH{};
	// rn::Program progBlurV{};

	rn::Prof profRender{"fx::CSM::profRender"};
	rn::Prof profBlur{"fx::CSM::profBlur"};

	std::string debugLog{};

	void init();

	void calculateMatrices(const ecs::Entity &cameraId, const ecs::Entity &lightId);
	void renderCascades();

	glm::vec2 findSceneZMinMax(glm::vec3 lightDirection);
	// void buildCorners(const glm::mat4 &VP, glm::vec4 (&output)[8]);
	// glm::mat4 buildShadowPMatrix(const glm::vec4 (&corners)[8], const glm::mat4 &V, float zMax);
	// glm::mat4 stabilizeVMatrix(const glm::mat4 &V, const glm::mat4 &P);
	// std::pair<glm::vec3, glm::vec3> computeBox(const glm::mat4 &splitProjection);
};

} // fx

#endif