#ifndef UTIL_INITQ_HPP
#define UTIL_INITQ_HPP

#include <deque>
#include <string>
#include <functional>

namespace util
{

class InitQ
{
public:
	bool autorun = false;
	std::string name = "Unnamed init queue";
	std::deque<std::function<void()>> queue{};

	InitQ() = default;
	explicit InitQ(std::string &&name);

	void run();

	void attach(std::function<void()> &&fn);
	void attachFirst(std::function<void()> &&fn);
};

class InitQAttacher
{
public:
	InitQAttacher(InitQ &container, std::function<void()> &&fn);
};

} // util

#endif