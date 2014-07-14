#include "timer.hpp"
#include "../ngn.hpp"

namespace util
{

void Timer::reset()
{
	updated = false;
	timed = 0.0;
	timef = 0.f;
}

void Timer::togglePause()
{
	paused = ! paused;
}

bool Timer::update()
{
	if ( ! updated)
	{
		prevTime = ngn::ct;
		updated = true;
	}

	if (paused)
	{
		prevTime = ngn::ct;
		return false;
	}

	timed += ngn::ct - prevTime;
	prevTime = ngn::ct;

	timef = static_cast<float>(timed);

	return false;
}

}