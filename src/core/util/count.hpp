#ifndef UTIL_COUNT_HPP
#define UTIL_COUNT_HPP

#include <array>

namespace util
{

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

} // util

#endif