#ifndef UTIL_HPP
#define UTIL_HPP

#include <array>

namespace util {

template<typename T, size_t N>
constexpr size_t countOf(const T (&)[N])
{
	return N;
}

template<typename T, size_t N>
constexpr size_t countOf(const std::array<T, N> (&))
{
	return N;
}

template <typename F>
class ScopeExit {
public:
	bool active = true;
	F fn;

	explicit ScopeExit(F fn)
	: fn{fn}
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
ScopeExit<F> makeScopeExit(F f) {
	return ScopeExit<F>{std::move(f)};
};

#define UTIL_CONCAT2_IMPL(one, two) one##two
#define UTIL_CONCAT2(one, two) UTIL_CONCAT2_IMPL(one, two)

#define UTIL_CONCAT3_IMPL(one, two, three) one##two##three
#define UTIL_CONCAT3(one, two, three) UTIL_CONCAT2_IMPL(one, two, three)

#define UTIL_SCOPE_EXIT(fn) \
	auto UTIL_CONCAT2(scopeExit, __COUNTER__) = util::makeScopeExit(fn)

#if defined(DEBUG) || defined (_DEBUG)
	#define UTIL_DEBUG
#else
	#define UTIL_DEBUG if (false)
#endif

} // util

#endif