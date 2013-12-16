#ifndef SRC_SBM_HPP
#define SRC_SBM_HPP

#include "../src.hpp"
#include "../gl/types.hpp"

#include <string>
#include <vector>

namespace src
{

namespace sbm
{

class Mesh : public src::Mesh
{
public:
	std::string fileName{};

	std::vector<char> dataCache{};

	Mesh(std::string &&fileName);
	void use();
	void release();
	std::string name();
};

std::unique_ptr<src::Mesh> mesh(const std::string &fileName);
std::unique_ptr<src::Mesh> mesh(std::string &&fileName);

} // sbm

} // src

#endif