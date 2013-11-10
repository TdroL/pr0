#ifndef SM_STATE_HPP
#define SM_STATE_HPP

#include "machine.hpp"

namespace sm
{

class State
{
public:
	Machine &machine;

	State(Machine &machine)
		: machine(machine)
	{}

	virtual void enter() = 0;
	virtual void run() = 0;
	virtual void pause() = 0;
	virtual void exit() = 0;

	virtual ~State() {}
};

}

#endif