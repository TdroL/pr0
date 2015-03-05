#ifndef UTIL_SCOPE_HPP
#define UTIL_SCOPE_HPP

#include "../util.hpp"

namespace util
{

template <typename F>
class ScopeExit
{
public:
	bool active = true;
	F fn;

	explicit ScopeExit(F &&fn)
		: fn{std::move(fn)}
	{}

	void dispatch()
	{
		if (active)
		{
			fn();
			active = false;
		}
	}

	~ScopeExit()
	{
		dispatch();
	}
};

template <typename F>
ScopeExit<F> makeScopeExit(F f)
{
	return ScopeExit<F>{std::move(f)};
}

#define UTIL_SCOPE_EXIT(fn) auto UTIL_CONCAT2(scopeExit, __COUNTER__) = util::makeScopeExit(fn)

} // util

#endif