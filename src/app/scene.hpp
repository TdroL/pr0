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

	void parseDirection(ecs::Entity &entity, const rapidjson::Value &member);
	void parseDirectionalLight(ecs::Entity &entity, const rapidjson::Value &member);
	void parseInput(ecs::Entity &entity, const rapidjson::Value &member);
	void parseMaterial(ecs::Entity &entity, const rapidjson::Value &member);
	void parseMesh(ecs::Entity &entity, const rapidjson::Value &member);
	void parseName(ecs::Entity &entity, const rapidjson::Value &member);
	void parseOccluder(ecs::Entity &entity, const rapidjson::Value &member);
	void parsePointLight(ecs::Entity &entity, const rapidjson::Value &member);
	void parsePosition(ecs::Entity &entity, const rapidjson::Value &member);
	void parseProjection(ecs::Entity &entity, const rapidjson::Value &member);
	void parseRotation(ecs::Entity &entity, const rapidjson::Value &member);
	void parseStencil(ecs::Entity &entity, const rapidjson::Value &member);
	void parseTemporalTransform(ecs::Entity &entity, const rapidjson::Value &member);
	void parseTransform(ecs::Entity &entity, const rapidjson::Value &member);
	void parseView(ecs::Entity &entity, const rapidjson::Value &member);
};

} // app

#endif