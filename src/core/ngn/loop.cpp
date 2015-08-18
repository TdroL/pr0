#include <pch.hpp>

#include "loop.hpp"
#include "../ngn.hpp"
#include <string>

namespace ngn
{

using namespace std;

int Loop::active = 0;

Loop::Loop(const char *file, int line)
{
	active++;

	if (active != 1)
	{
		throw string{"ngn::Loop::Loop() - multiple game loops ["} + file + ":" + to_string(line) + "]";
	}

	ngn::startLoop();
}

Loop::~Loop()
{
	ngn::endLoop();

	active--;
}

} // ngn