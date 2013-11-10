#ifndef SM_MACHINE_HPP
#define SM_MACHINE_HPP

#include <memory>

namespace sm
{

class State;

class Machine
{
public:
	std::map<int, std::unique_ptr<State>> states{};
	std::list<State *> active{};

	void enable(int id);
	void disable(int id);

	void activate(int id);
	void deactivate(int id);
};

}

#endif