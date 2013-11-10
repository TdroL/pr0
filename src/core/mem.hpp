#ifndef MEM_HPP
#define MEM_HPP

namespace mem
{

constexpr size_t alignTo = sizeof(void *);
size_t align(size_t size)
{
	return (size + (alignTo - 1)) & ~(alignTo - 1);
}

}

#endif