#ifndef APP_HPP
#define APP_HPP

#include <core/ecs/entity.hpp>

#include <core/rn/fb.hpp>
#include <core/rn/tb.hpp>
#include <core/rn/font.hpp>
#include <core/rn/mesh.hpp>
#include <core/rn/prof.hpp>
#include <core/rn/program.hpp>

#include <core/phs/frustum.hpp>

#include <app/fx/ssao.hpp>
#include <app/fx/csm.hpp>
#include <app/scene.hpp>

struct AppVariables;

class App
{
public:
	AppVariables *v = nullptr;

	App();
	~App();

	App(const App &) = delete;
	App operator=(const App &) = delete;

	App(App &&) = delete;
	App operator=(App &&) = delete;

	void init();

	void initProg();
	void initFB();
	void initScene();

	void update();

	void render();

	void zPrefillForwardPass();
	void setupLightsForwardPass();
	void renderShadowsForwardPass();
	void lightingForwardPass();

	void ssaoPass();
};

#endif