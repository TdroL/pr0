#ifndef UTIL_TIMER_HPP
#define UTIL_TIMER_HPP

namespace util
{

class Timer
{
public:
	bool updated = false;
	bool paused = false;

	double prevTime = 0.f;
	double timed = 0.f;
	float timef = 0.f;

	void reset();

	void togglePause();

	bool update(float rate = 1.f);
};

}

#endif