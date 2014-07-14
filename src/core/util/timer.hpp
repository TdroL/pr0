#ifndef UTIL_TIMER_HPP
#define UTIL_TIMER_HPP

namespace util
{

class Timer
{
public:
	bool updated = false;
	bool paused = false;

	double prevTime = 0.0;
	double timed = 0.0;
	float timef = 0.f;

	void reset();

	void togglePause();

	bool update();
};

}

#endif