#ifndef SRC_SBM_HPP
#define SRC_SBM_HPP

#include "../src.hpp"
#include "../rn/types.hpp"

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
	void open() override;
	void close() override;
	std::string name() override;
};

std::unique_ptr<src::Mesh> mesh(const std::string &fileName);
std::unique_ptr<src::Mesh> mesh(std::string &&fileName);

} // sbm

} // src

#endif