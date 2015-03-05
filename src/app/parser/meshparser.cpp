#include "meshparser.hpp"

#include <app/comp/mesh.hpp>

#include <core/asset/mesh.hpp>

#include <string>

namespace
{

using namespace std;

MeshParser meshParser{"mesh", [] (ecs::Entity &entity, const Parser::objectType &object)
{
	ecs::enable<comp::Mesh>(entity);

	auto &mesh = ecs::get<comp::Mesh>(entity);

	string idName;
	MeshParser::assign(object, "id", idName);

	mesh.id = asset::mesh::load(idName);
}};

}