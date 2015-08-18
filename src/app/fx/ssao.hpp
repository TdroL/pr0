#ifndef APP_FX_SSAO_HPP
#define APP_FX_SSAO_HPP

#include <app/comp/projection.hpp>

#include <core/rn.hpp>
#include <core/rn/fb.hpp>
#include <core/rn/program.hpp>
#include <core/rn/prof.hpp>

#include <string>
#include <vector>

namespace fx
{

class SSAO
{
public:
	GLfloat radius = 0.25;
	GLfloat intensity = 0.75;

	GLsizei zMipLevels = 5;

	comp::Projection projection{};

	rn::FB fbZ{"fx::SSAO::fbZ"};
	std::vector<rn::FB> fbZMipMaps{};
	rn::FB fbAO{"fx::SSAO::fbAO"};
	rn::FB fbBlur{"fx::SSAO::fbBlur"};

	rn::Program progReconstructZ{};
	rn::Program progMinify{};
	rn::Program progSAO{};
	rn::Program progBlur{};

	rn::Prof profZ{"fx::SSAO::profZ"};
	rn::Prof profMipMaps{"fx::SSAO::profMipMaps"};
	rn::Prof profAO{"fx::SSAO::profAO"};
	rn::Prof profBlur{"fx::SSAO::profBlur"};

	void init(const comp::Projection &projection);

	void clear();

	void genMipMaps(rn::FB &fbGBuffer);
	void computeAO(rn::FB &fbGBuffer);
	void blur(rn::FB &fbGBuffer);
};

} // fx

#endif