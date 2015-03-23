#ifndef APP_SCENE_HPP
#define APP_SCENE_HPP

#include <string>
#include <core/ecs/entity.hpp>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

namespace app
{

class Scene
{
public:
	std::string sceneSource{"scene.json"};

	ecs::Entity cameraId{0};

	void reload();
};

} // app

#endif