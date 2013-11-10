#ifndef SYS_LOOP_HPP
#define SYS_LOOP_HPP

#include "../util.hpp"

namespace sys
{

class Loop
{
public:
	static int active;

	Loop(const char *file, int line);

	~Loop();
};

#define SYS_LOOP sys::Loop UTIL_CONCAT2(loop, __COUNTER__)(__FILE__, __LINE__)

}

#endif