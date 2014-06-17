#ifndef APP_COMP_NAME_HPP
#define APP_COMP_NAME_HPP

#include <core/ecs/component.hpp>
#include <string>

namespace comp
{

struct Name : public ecs::Component<Name>
{
	std::string name{};
};

} // comp

#endif