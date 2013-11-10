#ifndef SG_OBJECT_HPP
#define SG_OBJECT_HPP

#include <glm/glm.hpp>

namespace sg
{

class Object
{
public:
	glm::vec3 position{0.0f, 0.0f, 0.0f};
	glm::vec3 rotation{0.0f, 0.0f, 0.0f};

	virtual void render() = 0;

	virtual ~Object() {}
};

}

#endif