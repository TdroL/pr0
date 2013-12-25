#ifndef UTIL_INITQ_HPP
#define UTIL_INITQ_HPP

#include <vector>
#include <functional>

namespace util {

class InitQ
{
public:
	bool autorun = false;
	std::vector<std::function<void()>> queue{};

	void run()
	{
		for (auto &fn : queue)
		{
			fn();
		}

		autorun = true;
	}
};

class InitQAttacher
{
public:
	InitQAttacher(InitQ &container, std::function<void()> &&fn)
	{
		container.queue.push_back(move(fn));

		if (container.autorun)
		{
			fn();
		}
	}
};

}

#endif