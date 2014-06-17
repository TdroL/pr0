#ifndef UTIL_ALIGN_HPP
#define UTIL_ALIGN_HPP

#include <cstddef>

namespace util
{

std::size_t align(std::size_t size, std::size_t alignTo);
std::size_t align(std::size_t size);

template<typename T>
T nextPowerOf2(T n)
{
	T k = 1;

	while (k < n)
	{
		k *= 2;
	}

	return k;
}

}

#endif