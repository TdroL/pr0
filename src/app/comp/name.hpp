#pragma once

#include <core/ecs/component.hpp>
#include <string>

namespace comp
{

struct Name : public ecs::Component<Name>
{
	std::string name{};
};

} // comp