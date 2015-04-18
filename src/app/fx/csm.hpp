#ifndef APP_FX_CSM_HPP
#define APP_FX_CSM_HPP

#include <app/comp/projection.hpp>

#include <core/rn/fb.hpp>
#include <core/rn/prof.hpp>
#include <core/rn/program.hpp>
#include <core/rn/tex2darray.hpp>
#include <core/ecs/entity.hpp>

#include <vector>

#include <glm/glm.hpp>

namespace fx
{

class CSM
{
public:
	std::vector<float> splits{ 0.05f, 0.15f, 0.5f, 1.f };
	std::vector<float> cascades{};

	std::shared_ptr<rn::Tex2DArray> texCascades{};
	std::shared_ptr<rn::Tex2DArray> texDepths{};
	std::vector<rn::FB> fbShadows{};
	rn::FB fbBlurBuffer{};

	std::vector<glm::mat4> Ps{};
	glm::mat4 V{};

	rn::Program progDepth{};
	rn::Program progBlurH{};
	rn::Program progBlurV{};

	rn::Prof profRender{"fx::CSM::profRender"};
	rn::Prof profBlur{"fx::CSM::profBlur"};

	ecs::Entity cameraId{};

	void init();

	void setup(const ecs::Entity &lightId);
	void render();

	void buildCorners(const glm::mat4 &VP, glm::vec4 (&output)[8]);
	glm::mat4 buildShadowPMatrix(const glm::vec4 (&corners)[8], float zMax);
	float findSceneZMax(const ecs::Entity &lightId);

};

} // fx

#endif