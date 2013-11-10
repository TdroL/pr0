#ifndef MEM_LOOP_HPP
#define MEM_LOOP_HPP

#include <cstddef>
#include <memory>

namespace mem
{

namespace loop
{

void init();

void free(void *ptr);

void * alloc(size_t size);

} // loop

} // mem

#endif