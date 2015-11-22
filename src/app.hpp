#pragma once

#include <memory>

class App
{
public:
	class Variables;

	std::unique_ptr<Variables> v;

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