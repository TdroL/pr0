#include "loop.hpp"
#include <memory>
#include <algorithm>
#include <cassert>
#include <iostream>
#include "../mem.hpp"
#include "../util/align.hpp"

namespace mem
{

namespace loop
{

using namespace std;

uint8_t *currentPtr = nullptr;
uint8_t *endPtr = nullptr;
size_t poolSize = 20; // 20 * 1024 * 1024 = 20 MiB
unique_ptr<uint8_t[]> pool{};

void init()
{
	assert(endPtr >= currentPtr);

	if ( ! pool || poolSize > static_cast<decltype(poolSize)>(endPtr - currentPtr))
	{
		poolSize = util::align(poolSize);
		pool.reset(); // release first
		pool.reset(new uint8_t[poolSize]);
	}

	currentPtr = pool.get();
	endPtr = pool.get() + poolSize;

	clog << "Init loop memory: " << poolSize << " bytes (aligned to " << sizeof(void *) << ")" << endl;
}

void * alloc(size_t size)
{
	// align
	size = util::align(size);

	clog << "Allocating " << size << " bytes" << endl;

	uint8_t *returnPtr = currentPtr;
	currentPtr += size;

	if (currentPtr < endPtr)
	{
		return returnPtr;
	}

	poolSize += util::align(currentPtr - endPtr);
	clog << "Pool overflow, requesting pool resize to " << poolSize << " bytes" << endl;

	currentPtr -= size;

	return new uint8_t[size];
}

void free(void *ptr)
{
	if (ptr < pool.get() || ptr > endPtr)
	{
		delete[] reinterpret_cast<uint8_t *>(ptr);
	}
}

} // loop

} // mem