#include "loop.hpp"
#include "../sys.hpp"
#include <string>

namespace sys
{

using namespace std;

int Loop::active = 0;

Loop::Loop(const char *file, int line)
{
	active++;

	if (active != 1)
	{
		throw string{"sys::Loop::Loop() - multiple system loops ["} + file + ":" + to_string(line) + "]";
	}

	sys::startLoop();
}

Loop::~Loop()
{
	sys::endLoop();

	active--;
}


}