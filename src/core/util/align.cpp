#include "align.hpp"

namespace util
{

using namespace std;

std::size_t align(std::size_t size, std::size_t alignTo)
{
	return (size + (alignTo - 1)) & ~(alignTo - 1);
}

std::size_t align(std::size_t size)
{
	return util::align(size, sizeof(void *));
}

}