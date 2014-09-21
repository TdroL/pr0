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
	timef = static_cast<float>(timed);
	prevTime = ngn::ct;


	return false;
}

}