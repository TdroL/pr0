#ifndef RN_TIMER_HPP
#define RN_TIMER_HPP

#include "../rn.hpp"
#include "../util.hpp"
#include <string>
#include <vector>

namespace rn
{

class Prof
{
public:
	static std::vector<Prof *> collection;
	static void reloadSoftAll();
	static void reloadAll();
	static void swapAll();

	static constexpr size_t maxQueries = 10;

	std::string profName = "Unnamed query timer";

	enum {
		START = 0,
		STOP = 1
	};

	GLuint queries[maxQueries][2] { { 0 } }; // { front { start, stop }, back { start, stop } }
	// GLuint64 times[2] { 0 }; // { front, back }

	bool active = false;

	GLuint64 dt = 0; // delta time
	int df = 0; // delta frames (delay)

	int front = 0;
	int back = 0;

	Prof();
	explicit Prof(std::string &&profName);
	~Prof();

	void init();

	void reset();

	void reloadSoft();
	void reload();

	void swap();

	void start();
	void stop();

	int latency();
	GLuint64 ns();
	double ms();
};

class ProfScoper
{
public:
	Prof &prof;

	ProfScoper(Prof &prof) : prof(prof) { prof.start(); }
	~ProfScoper() { prof.stop(); }
};

#define RN_PROF_RUN(profSource) rn::ProfScoper UTIL_CONCAT2(ProfScoper, __COUNTER__)(profSource)

} // rn

#endif