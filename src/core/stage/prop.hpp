#ifndef STAGE_PROP_HPP
#define STAGE_PROP_HPP

#include "../gl/mesh.hpp"

#include <string>
#include <glm/glm.hpp>

namespace stage
{

class Prop
{
public:
	glm::vec3 position{};
	glm::vec3 rotation{};
	gl::Mesh mesh{};

	std::string propName = "Unknown prop";

	Prop() = default;
	Prop(std::string &&propName)
		: propName{std::move(propName)}
	{}
};

} // stage

#endif