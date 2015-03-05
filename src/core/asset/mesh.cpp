#include "mesh.hpp"

#include <iostream>

#include "../src/obj.hpp"
#include "../src/sbm.hpp"

namespace asset
{

namespace mesh
{

using namespace std;

vector<rn::Mesh> meshes{};
map<string, size_t> mappings{};

size_t add(const string &name, rn::Mesh &&mesh)
{
	auto it = mappings.find(name);
	size_t id = 0;

	if (it != end(mappings))
	{
		id = it->second;

		meshes[id - 1] = move(mesh);
	}
	else
	{
		meshes.push_back(move(mesh));
		id = meshes.size();
		mappings[name] = id;
	}

	return id;
}

size_t load(const string &name)
{
	const auto it = mappings.find(name);
	size_t id = 0;

	if (it != end(mappings))
	{
		id = it->second;
	}
	else
	{
		rn::Mesh mesh{string{name}};

		if (name.compare(name.size() - 4, 4, ".obj") == 0)
		{
			mesh.load(src::obj::mesh(name));
		}
		else if (name.compare(name.size() - 4, 4, ".sbm") == 0)
		{
			mesh.load(src::sbm::mesh(name));
		}
		else
		{
			throw string{"asset::mesh::load() - unknown mesh format: " + name};
		}

		id = add(name, move(mesh));
	}

	return id;
}

rn::Mesh & get(const size_t id)
{
	if (id == 0 || meshes.size() < id)
	{
		throw string{"asset::mesh::get() - invalid id (" + to_string(id) + ")"};
	}

	return meshes[id - 1];
}

} // mesh

} // asset