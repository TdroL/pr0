#ifndef APP_FX_CSM_HPP
#define APP_FX_CSM_HPP

// #include <pch.hpp>

#include <app/comp/directionallight.hpp>
#include <app/comp/projection.hpp>
#include <app/comp/view.hpp>

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
	static const size_t textureResolution = 2 * 1024;

	unsigned int kernelSize = 7;
	size_t splits = 5;
	bool blendCascades = true;
	float maxShadowDistance = 256.f;

	std::shared_ptr<rn::Tex2DArray> texDepths{};
	std::vector<rn::FB> fbShadows{};

	rn::Program progDepth{"fx::CSM::progDepth"};
	// rn::Prof profRender{"fx::CSM::profRender"};

	std::vector<float> radiuses2{};
	std::vector<glm::mat4> Ps{};
	std::vector<glm::mat4> Vs{};
	std::vector<glm::mat4> shadowBiasedVPs{};
	std::vector<glm::vec3> centers{};

	void init();

	void calculateMatrices(const comp::DirectionalLight &light, const comp::Projection &projection, const comp::View &view, float zMax);
};

} // fx

#endif