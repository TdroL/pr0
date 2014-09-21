#include "initq.hpp"

#include <iostream>

namespace util
{

InitQ::InitQ(std::string &&name)
{
	if ( ! name.empty())
	{
		this->name = std::move(name);
	}
}

void InitQ::run()
{
	for (auto &fn : queue)
	{
		fn();
	}

	autorun = true;
}

void InitQ::attach(std::function<void()> &&fn)
{
	queue.push_back(move(fn));

	if (autorun)
	{
		queue.back()();
	}
}

InitQAttacher::InitQAttacher(InitQ &container, std::function<void()> &&fn)
{
	container.attach(move(fn));
}

} // util