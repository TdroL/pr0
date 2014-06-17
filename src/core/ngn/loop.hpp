#ifndef NGN_LOOP_HPP
#define NGN_LOOP_HPP

#include "../util.hpp"

namespace ngn
{

class Loop
{
public:
	static int active;

	Loop(const char *file, int line);

	~Loop();
};

#define NGN_LOOP ngn::Loop UTIL_CONCAT2(loop, __COUNTER__)(__FILE__, __LINE__)

}

#endif