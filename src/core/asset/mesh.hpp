#ifndef ASSET_MESH_HPP
#define ASSET_MESH_HPP

#include "../gl/mesh.hpp"
#include <string>
#include <vector>
#include <map>

namespace asset
{

namespace mesh
{

extern std::vector<gl::Mesh> meshes;
extern std::map<std::string, size_t> mappings;

size_t add(const std::string &name, gl::Mesh &&mesh);
size_t load(const std::string &name);
gl::Mesh & get(const size_t id);

} // mesh

} // asset

#endif