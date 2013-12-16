#ifndef SRC_OBJ_HPP
#define SRC_OBJ_HPP

#include "../src.hpp"
#include "../gl/types.hpp"

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
	void use();
	void release();
	std::string name();
};

std::unique_ptr<src::Mesh> mesh(const std::string &fileName);
std::unique_ptr<src::Mesh> mesh(std::string &&fileName);

} // obj

} // src

#endif