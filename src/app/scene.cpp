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
#include <app/comp/stencil.hpp>
#include <app/comp/temporaltransform.hpp>
#include <app/comp/transform.hpp>
#include <app/comp/view.hpp>

// #include <app/parser/directionparser.hpp>
// #include <app/parser/directionallightparser.hpp>
// #include <app/parser/inputparser.hpp>
#include <app/parser/materialparser.hpp>
#include <app/parser/meshparser.hpp>
#include <app/parser/nameparser.hpp>
#include <app/parser/occluderparser.hpp>
// #include <app/parser/pointlightparser.hpp>
// #include <app/parser/positionparser.hpp>
// #include <app/parser/projectionparser.hpp>
// #include <app/parser/rotationparser.hpp>
// #include <app/parser/stencilparser.hpp>
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

	for (rapidjson::SizeType i = 0; i < scene.Size(); i++) // rapidjson uses SizeType instead of size_t.
	{
		// cout << "-----" << endl;
		const rapidjson::Value &object = scene[i];

		if ( ! object.IsObject())
		{
			continue;
		}

		ecs::Entity entity = ecs::create();

		ecs::enable<CreatedBySceneLoader>(entity);

		parseDirection(entity, object);
		parseDirectionalLight(entity, object);
		parseInput(entity, object);
		// parseMaterial(entity, object);
		// parseMesh(entity, object);
		// parseName(entity, object);
		// parseOccluder(entity, object);
		parsePointLight(entity, object);
		parsePosition(entity, object);
		parseProjection(entity, object);
		parseRotation(entity, object);
		parseStencil(entity, object);
		parseTemporalTransform(entity, object);
		parseTransform(entity, object);
		parseView(entity, object);


		Parser::parse(entity, object);
	}

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [Scene:" << (ngn::time() - timer) << "s]" << endl;
		clog.unsetf(ios::floatfield);
	}
}

void Scene::parseDirection(ecs::Entity &, const rapidjson::Value &)
{}

void Scene::parseDirectionalLight(ecs::Entity &, const rapidjson::Value &)
{}

void Scene::parseInput(ecs::Entity &entity, const rapidjson::Value &member)
{
	if (member.HasMember("input"))
	{
		// cout << "input";
		ecs::enable<Input>(entity);

		const auto &inputMember = member["input"];

		if (inputMember.IsObject())
		{
			auto &input = ecs::get<Input>(entity);

			if (inputMember.HasMember("keyCommands"))
			{
				const auto &keyCommandsInputMember = inputMember["keyCommands"];

				if (keyCommandsInputMember.IsArray())
				{
					for (rapidjson::SizeType i = 0; i < keyCommandsInputMember.Size(); i++)
					{
						const auto &keyCommand = keyCommandsInputMember[i];

						if ( ! keyCommand.IsArray() || keyCommand.Size() < 2)
						{
							continue;
						}

						const auto &keyName = keyCommand[0];
						const auto &commandName = keyCommand[1];

						if ( ! keyName.IsString() || ! commandName.IsString())
						{
							continue;
						}

						size_t key = 0;
						bitset<Input::KEYSTATE_ALL> onKeyState{0};

						const string &keyNameString = keyName.GetString();

						if (keyNameString.length() > 1)
						{
							char mod = keyNameString[0];

							switch (mod)
							{
								case '*':
								{
									onKeyState.set(Input::KEYSTATE_HOLD);
									key = ngn::key::getKey(keyNameString.substr(1));
									break;
								}
								case '-':
								{
									onKeyState.set(Input::KEYSTATE_RELEASE);
									key = ngn::key::getKey(keyNameString.substr(1));
									break;
								}
								case '/':
								{
									onKeyState.set(Input::KEYSTATE_PRESS);
									onKeyState.set(Input::KEYSTATE_RELEASE);
									key = ngn::key::getKey(keyNameString.substr(1));
									break;
								}
								default:
								{
									onKeyState.set(Input::KEYSTATE_PRESS);
									key = ngn::key::getKey(keyNameString);
								}
							}
						}
						else
						{
							onKeyState.set(Input::KEYSTATE_PRESS);
							key = ngn::key::getKey(keyNameString);
						}

						string command = commandName.GetString();
						util::str::lowercase(command);

						if (command == "moveforward")  { input.keyCommands.emplace_back(key, onKeyState, Input::COMMAND_MOVEFORWARD); continue; }
						if (command == "movebackward") { input.keyCommands.emplace_back(key, onKeyState, Input::COMMAND_MOVEBACKWARD); continue; }
						if (command == "strafeleft")   { input.keyCommands.emplace_back(key, onKeyState, Input::COMMAND_STRAFELEFT); continue; }
						if (command == "straferight")  { input.keyCommands.emplace_back(key, onKeyState, Input::COMMAND_STRAFERIGHT); continue; }
						if (command == "moveupward")   { input.keyCommands.emplace_back(key, onKeyState, Input::COMMAND_MOVEUPWARD); continue; }
						if (command == "movedownward") { input.keyCommands.emplace_back(key, onKeyState, Input::COMMAND_MOVEDOWNWARD); continue; }
					}
				}
			}
		}
		// cout << endl;
	}
}

void Scene::parseMaterial(ecs::Entity &entity, const rapidjson::Value &member)
{
	if (member.HasMember("material"))
	{
		// cout << "material";
		ecs::enable<Material>(entity);

		const auto &materialMember = member["material"];

		if (materialMember.IsObject())
		{
			auto &material = ecs::get<Material>(entity);

			if (materialMember.HasMember("diffuse"))
			{
				const auto &diffuseMaterialMember = materialMember["diffuse"];

				if (diffuseMaterialMember.IsArray())
				{
					if (diffuseMaterialMember.Size() == 1)
					{
						material.diffuse = glm::vec4{
							diffuseMaterialMember[0].IsNumber() ? static_cast<float>(diffuseMaterialMember[0].GetDouble()) : 0.f
						};
					}
					else if (diffuseMaterialMember.Size() == 4)
					{
						material.diffuse = glm::vec4{
							diffuseMaterialMember[0].IsNumber() ? static_cast<float>(diffuseMaterialMember[0].GetDouble()) : 0.f,
							diffuseMaterialMember[1].IsNumber() ? static_cast<float>(diffuseMaterialMember[1].GetDouble()) : 0.f,
							diffuseMaterialMember[2].IsNumber() ? static_cast<float>(diffuseMaterialMember[2].GetDouble()) : 0.f,
							diffuseMaterialMember[3].IsNumber() ? static_cast<float>(diffuseMaterialMember[3].GetDouble()) : 0.f
						};
					}
				}
			}

			if (materialMember.HasMember("shininess"))
			{
				const auto &shininessMaterialMember = materialMember["shininess"];

				if (shininessMaterialMember.IsNumber())
				{
					material.shininess = shininessMaterialMember.GetDouble();
				}
			}
		}
		// cout << endl;
	}
}

void Scene::parseMesh(ecs::Entity &entity, const rapidjson::Value &member)
{
	if (member.HasMember("mesh"))
	{
		// cout << "mesh";
		ecs::enable<Mesh>(entity);

		const auto &meshMember = member["mesh"];

		if (meshMember.IsObject())
		{
			if (meshMember.HasMember("id"))
			{
				auto &mesh = ecs::get<Mesh>(entity);

				const auto &idMeshMember = meshMember["id"];

				if (idMeshMember.IsString())
				{
					mesh.id = asset::mesh::load(idMeshMember.GetString());
					// cout << " [" << mesh.id << ":" << idMeshMember.GetString() << "]";
				}
			}
		}
		// cout << endl;
	}
}

void Scene::parseName(ecs::Entity &entity, const rapidjson::Value &member)
{
	if (member.HasMember("name"))
	{
		// cout << "name";
		ecs::enable<Name>(entity);

		const auto &nameMember = member["name"];

		if (nameMember.IsObject())
		{
			if (nameMember.HasMember("name"))
			{
				auto &name = ecs::get<Name>(entity);

				const auto &nameNameMember = nameMember["name"];

				if (nameNameMember.IsString())
				{
					name.name = nameNameMember.GetString();

					// cout << " [" << name.name << "]";
				}
			}
		}
		// cout << endl;
	}
}

void Scene::parseOccluder(ecs::Entity &entity, const rapidjson::Value &member)
{
	if (member.HasMember("occluder"))
	{
		// cout << "occluder";
		ecs::enable<Occluder>(entity);
		// cout << endl;
	}
}

void Scene::parsePointLight(ecs::Entity &, const rapidjson::Value &)
{}

void Scene::parsePosition(ecs::Entity &, const rapidjson::Value &)
{}

void Scene::parseProjection(ecs::Entity &, const rapidjson::Value &)
{}

void Scene::parseRotation(ecs::Entity &, const rapidjson::Value &)
{}

void Scene::parseStencil(ecs::Entity &entity, const rapidjson::Value &member)
{
	if (member.HasMember("stencil"))
	{
		// cout << "stencil";
		ecs::enable<Stencil>(entity);

		const auto &stencilMember = member["stencil"];

		if (stencilMember.IsObject())
		{
			if (stencilMember.HasMember("ref"))
			{
				auto &stencil = ecs::get<Stencil>(entity);

				const auto &refStencilMember = stencilMember["ref"];

				if (refStencilMember.IsString())
				{
					const string refValue = refStencilMember.GetString();

					if (refValue == "shaded")
					{
						stencil.ref = Stencil::MASK_SHADED;
					}
					else if (refValue == "flat")
					{
						stencil.ref = Stencil::MASK_FLAT;
					}
				}
				else if (refStencilMember.IsUint())
				{
					stencil.ref = refStencilMember.GetUint();
				}
			}
		}
		// cout << endl;
	}
}

void Scene::parseTransform(ecs::Entity &entity, const rapidjson::Value &member)
{
	if (member.HasMember("transform"))
	{
		// cout << "transform";
		ecs::enable<Transform>(entity);

		const auto &transformMember = member["transform"];

		if (transformMember.IsObject())
		{
			auto &transform = ecs::get<Transform>(entity);

			if (transformMember.HasMember("translation"))
			{
				const auto &translationTransformMember = transformMember["translation"];

				if (translationTransformMember.IsArray())
				{
					if (translationTransformMember.Size() == 1)
					{
						transform.translation = glm::vec3{
							translationTransformMember[0].IsNumber() ? static_cast<float>(translationTransformMember[0].GetDouble()) : 0.f
						};

					}
					else if (translationTransformMember.Size() == 3)
					{
						transform.translation = glm::vec3{
							translationTransformMember[0].IsNumber() ? static_cast<float>(translationTransformMember[0].GetDouble()) : 0.f,
							translationTransformMember[1].IsNumber() ? static_cast<float>(translationTransformMember[1].GetDouble()) : 0.f,
							translationTransformMember[2].IsNumber() ? static_cast<float>(translationTransformMember[2].GetDouble()) : 0.f
						};

					}
				}
			}

			if (transformMember.HasMember("rotation"))
			{
				const auto &rotationTransformMember = transformMember["rotation"];

				if (rotationTransformMember.IsArray())
				{
					if (rotationTransformMember.Size() == 1)
					{
						transform.rotation = glm::vec3{
							rotationTransformMember[0].IsNumber() ? static_cast<float>(rotationTransformMember[0].GetDouble()) : 0.f
						};

					}
					else if (rotationTransformMember.Size() == 3)
					{
						transform.rotation = glm::vec3{
							rotationTransformMember[0].IsNumber() ? static_cast<float>(rotationTransformMember[0].GetDouble()) : 0.f,
							rotationTransformMember[1].IsNumber() ? static_cast<float>(rotationTransformMember[1].GetDouble()) : 0.f,
							rotationTransformMember[2].IsNumber() ? static_cast<float>(rotationTransformMember[2].GetDouble()) : 0.f
						};

					}
				}
			}

			if (transformMember.HasMember("scale"))
			{
				const auto &scaleTransformMember = transformMember["scale"];

				if (scaleTransformMember.IsArray())
				{
					if (scaleTransformMember.Size() == 1)
					{
						transform.scale = glm::vec3{
							scaleTransformMember[0].IsNumber() ? static_cast<float>(scaleTransformMember[0].GetDouble()) : 0.f
						};

					}
					else if (scaleTransformMember.Size() == 3)
					{
						transform.scale = glm::vec3{
							scaleTransformMember[0].IsNumber() ? static_cast<float>(scaleTransformMember[0].GetDouble()) : 0.f,
							scaleTransformMember[1].IsNumber() ? static_cast<float>(scaleTransformMember[1].GetDouble()) : 0.f,
							scaleTransformMember[2].IsNumber() ? static_cast<float>(scaleTransformMember[2].GetDouble()) : 0.f
						};

					}
				}
			}
		}
		// cout << endl;
	}
}

void Scene::parseTemporalTransform(ecs::Entity &entity, const rapidjson::Value &member)
{
	if (member.HasMember("temporalTransform"))
	{
		// cout << "temporalTransform";
		ecs::enable<TemporalTransform>(entity);

		const auto &temporalTransformMember = member["temporalTransform"];

		if (temporalTransformMember.IsObject())
		{
			auto &temporalTransform = ecs::get<TemporalTransform>(entity);

			if (temporalTransformMember.HasMember("translationSpeed"))
			{
				const auto &translationSpeedTemporalTransformMember = temporalTransformMember["translationSpeed"];

				if (translationSpeedTemporalTransformMember.IsArray())
				{
					if (translationSpeedTemporalTransformMember.Size() == 1)
					{
						temporalTransform.translationSpeed = glm::vec3{
							translationSpeedTemporalTransformMember[0].IsNumber() ? static_cast<float>(translationSpeedTemporalTransformMember[0].GetDouble()) : 0.f
						};

					}
					else if (translationSpeedTemporalTransformMember.Size() == 3)
					{
						temporalTransform.translationSpeed = glm::vec3{
							translationSpeedTemporalTransformMember[0].IsNumber() ? static_cast<float>(translationSpeedTemporalTransformMember[0].GetDouble()) : 0.f,
							translationSpeedTemporalTransformMember[1].IsNumber() ? static_cast<float>(translationSpeedTemporalTransformMember[1].GetDouble()) : 0.f,
							translationSpeedTemporalTransformMember[2].IsNumber() ? static_cast<float>(translationSpeedTemporalTransformMember[2].GetDouble()) : 0.f
						};

					}
				}
			}

			if (temporalTransformMember.HasMember("translationNormalized"))
			{
				const auto &translationNormalizedTemporalTransformMember = temporalTransformMember["translationNormalized"];

				temporalTransform.translationNormalized = translationNormalizedTemporalTransformMember.IsTrue();
			}

			if (temporalTransformMember.HasMember("rotationSpeed"))
			{
				const auto &rotationSpeedTemporalTransformMember = temporalTransformMember["rotationSpeed"];

				if (rotationSpeedTemporalTransformMember.IsArray())
				{
					if (rotationSpeedTemporalTransformMember.Size() == 1)
					{
						temporalTransform.rotationSpeed = glm::vec3{
							rotationSpeedTemporalTransformMember[0].IsNumber() ? static_cast<float>(rotationSpeedTemporalTransformMember[0].GetDouble()) : 0.f
						};

					}
					else if (rotationSpeedTemporalTransformMember.Size() == 3)
					{
						temporalTransform.rotationSpeed = glm::vec3{
							rotationSpeedTemporalTransformMember[0].IsNumber() ? static_cast<float>(rotationSpeedTemporalTransformMember[0].GetDouble()) : 0.f,
							rotationSpeedTemporalTransformMember[1].IsNumber() ? static_cast<float>(rotationSpeedTemporalTransformMember[1].GetDouble()) : 0.f,
							rotationSpeedTemporalTransformMember[2].IsNumber() ? static_cast<float>(rotationSpeedTemporalTransformMember[2].GetDouble()) : 0.f
						};

					}
				}
			}
		}
		// cout << endl;
	}
}

void Scene::parseView(ecs::Entity &, const rapidjson::Value &)
{}


} // app