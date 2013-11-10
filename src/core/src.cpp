#include "src.hpp"

namespace src
{

std::unique_ptr<Mesh> cast(std::unique_ptr<Mesh> &&mesh)
{
	return std::unique_ptr<Mesh>{std::move(mesh)};
}

} // src