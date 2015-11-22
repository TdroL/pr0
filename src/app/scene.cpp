#include <pch.hpp>

#include "scene.hpp"

#include <core/ngn.hpp>
#include <core/ngn/fs.hpp>
#include <core/ngn/key.hpp>
#include <core/util/str.hpp>
#include <core/asset/mesh.hpp>

#include <app/comp/createdbysceneloader.hpp>
#include <app/comp/direction.hpp>
#include <app/comp/directionallight.hpp>
#include <app/comp/input.hpp>
#include <app/comp/material.hpp>
#include <app/comp/mesh.hpp>
#include <app/comp/name.hpp>
#include <app/comp/occluder.hpp>
#include <app/comp/pointlight.hpp>
#include <app/comp/position.hpp>
#include <app/comp/projection.hpp>
#include <app/comp/rotation.hpp>
#include <app/comp/temporaltransform.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/view.hpp>

// #include <app/parser/directionparser.hpp>
// #include <app/parser/directionallightparser.hpp>
#include <app/parser/inputparser.hpp>
#include <app/parser/materialparser.hpp>
#include <app/parser/meshparser.hpp>
#include <app/parser/nameparser.hpp>
#include <app/parser/occluderparser.hpp>
#include <app/parser/pointlightparser.hpp>
// #include <app/parser/positionparser.hpp>
// #include <app/parser/projectionparser.hpp>
// #include <app/parser/rotationparser.hpp>
#include <app/parser/shadingparser.hpp>
// #include <app/parser/temporaltransformparser.hpp>
#include <app/parser/transformparser.hpp>
// #include <app/parser/viewparser.hpp>

#include <iostream>

namespace app
{

namespace fs = ngn::fs;

using namespace std;
using namespace comp;

void Scene::reload()
{
	double timer = ngn::time();

	string sceneContents = fs::contents("scene.json");

	rapidjson::Document scene{};
	scene.Parse(sceneContents.c_str());

	if (scene.HasParseError())
	{
		throw string{"Parse errors"};
	}

	if ( ! scene.IsArray())
	{
		throw string{"Incorrect scene.json data"};
	}

	for (auto &entity : ecs::findWith<CreatedBySceneLoader>())
	{
		ecs::destroy(entity);
	}

	for (rapidjson::SizeType i = 0; i < scene.Size(); i++)
	{
		const rapidjson::Value &object = scene[i];

		if ( ! object.IsObject())
		{
			continue;
		}

		ecs::Entity entity = ecs::create();
		ecs::enable<CreatedBySceneLoader>(entity);

		Parser::parse(entity, object);
	}

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [Scene:" << (ngn::time() - timer) << "s]" << endl;
		clog.unsetf(ios::floatfield);
	}
}

} // app