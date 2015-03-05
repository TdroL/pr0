#ifndef UTIL_TOGGLE_HPP
#define UTIL_TOGGLE_HPP

#include <vector>
#include <string>

namespace util
{

class Toggle
{
public:
	static std::vector<Toggle *> collection;

	unsigned int value = 0;
	unsigned int range = 2; // [0, range)

	std::string toggleName = "Unnamed toggle";

	Toggle();
	Toggle(std::string &&toggleName, unsigned int value = 0, unsigned int range = 2);

	~Toggle();

	unsigned int change();
	void reset();

};

}

#endif