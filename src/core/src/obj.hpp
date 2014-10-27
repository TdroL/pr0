#ifndef SRC_OBJ_HPP
#define SRC_OBJ_HPP

#include "../src.hpp"
#include "../rn/types.hpp"

#include <string>
#include <vector>

namespace src
{

namespace obj
{

class Mesh : public src::Mesh
{
public:
	std::string fileName{};

	std::vector<GLfloat> vertexCache{};
	std::vector<GLuint> indexCache{};

	Mesh(std::string &&fileName);
	void open() override;
	void close() override;
	std::string name() override;
};

std::unique_ptr<src::Mesh> mesh(const std::string &fileName);
std::unique_ptr<src::Mesh> mesh(std::string &&fileName);

} // obj

} // src

#endif