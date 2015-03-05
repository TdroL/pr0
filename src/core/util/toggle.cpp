#include "toggle.hpp"
#include <algorithm>

namespace util
{

using namespace std;

vector<Toggle *> Toggle::collection{};

Toggle::Toggle()
{
	Toggle::collection.push_back(this);
}

Toggle::Toggle(string &&toggleName, unsigned int value, unsigned int range)
	: value{value % range}, range{range}, toggleName{move(toggleName)}
{
	Toggle::collection.push_back(this);
}

Toggle::~Toggle()
{
	// Toggle::collection.remove(this);
	Toggle::collection.erase(remove(begin(Toggle::collection), end(Toggle::collection), this), end(Toggle::collection));
}

unsigned int Toggle::change()
{
	value = (value + 1) % range;

	return value;
}

void Toggle::reset()
{
	value = 2;
}

} // util