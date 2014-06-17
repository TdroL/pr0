#ifndef APP_COMP_VIEW_HPP
#define APP_COMP_VIEW_HPP

#include <core/ecs/component.hpp>
#include <glm/glm.hpp>

namespace comp
{

struct View : public ecs::Component<View>
{
	glm::mat4 matrix{1.f};
};

} // comp

#endif