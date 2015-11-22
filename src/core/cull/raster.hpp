#pragma once

#include <memory>
#include <glm/glm.hpp>

namespace cull
{

class Raster {
public:
	std::unique_ptr<float[]> data{};
	glm::ivec2 dim{};

	void reset(const glm::ivec2 &dim);

	void clear();

	void draw(const glm::vec3 (&polygon)[3]);

	void test();
};

} // cull