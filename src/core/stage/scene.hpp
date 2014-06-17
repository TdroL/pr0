#ifndef STAGE_SCENE_HPP
#define STAGE_SCENE_HPP

#include "prop.hpp"

#include <map>

namespace stage
{

class Scene
{
public:
	std::map<int, stage::Prop> props{};

	void render();
};

} // stage

#endif