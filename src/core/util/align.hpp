#ifndef UTIL_ALIGN_HPP
#define UTIL_ALIGN_HPP

namespace util
{

size_t align(size_t size, size_t alignTo)
{
	return (size + (alignTo - 1)) & ~(alignTo - 1);
}

size_t align(size_t size)
{
	return util::align(size, sizeof(void *));
}

}

#endif