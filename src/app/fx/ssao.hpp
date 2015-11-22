#ifndef APP_FX_SSAO_HPP
#define APP_FX_SSAO_HPP

#include <app/events.hpp>
#include <app/comp/projection.hpp>

#include <core/rn.hpp>
#include <core/rn/fb.hpp>
#include <core/rn/tex.hpp>
#include <core/rn/program.hpp>
#include <core/rn/prof.hpp>

#include <core/event.hpp>
#include <core/ngn/window.hpp>

#include <string>
#include <vector>

namespace fx
{

class SSAO
{
public:
	bool dirty = false;

	GLfloat radius = 0.25;
	GLfloat intensity = 0.75;

	GLsizei zMipLevels = 5;

	comp::Projection projection{};

	rn::FB fbZ{"fx::SSAO::fbZ"};
	rn::FB fbAO{"fx::SSAO::fbAO"};
	rn::FB fbBlur{"fx::SSAO::fbBlur"};

	std::vector<rn::FB> fbZMipMaps{};

	rn::Program progReconstructZ{"fx::SSAO::progReconstructZ"};
	rn::Program progMinify{"fx::SSAO::progMinify"};
	rn::Program progSAO{"fx::SSAO::progSAO"};
	rn::Program progBlur{"fx::SSAO::progBlur"};

	rn::Prof profZ{"fx::SSAO::profZ"};
	rn::Prof profMipMaps{"fx::SSAO::profMipMaps"};
	rn::Prof profAO{"fx::SSAO::profAO"};
	rn::Prof profBlur{"fx::SSAO::profBlur"};

	event::Listener<ngn::window::WindowResizeEvent> listenerWindowResize{};
	event::Listener<ProjectionChangedEvent> listenerProjectionChanged{};

	void init(const comp::Projection &projection);

	void clear();

	void genMipMaps(const rn::Tex &texDepth);
	void computeAO(const rn::Tex &texNormal);
	void blur();
};

} // fx

#endif