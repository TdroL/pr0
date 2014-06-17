#ifndef ECS_COMPONENT_HPP
#define ECS_COMPONENT_HPP

#include "../ecs.hpp"

#include <vector>
#include <string>
#include <bitset>
#include <cassert>

namespace ecs
{

extern int bitCounter;

template<typename C>
class Component
{
public:
	static int bit()
	{
		static int bit = bitCounter++;

		assert(bit < ecs::MAX_COMPONENTS);

		return bit;
	}

	static std::vector<C> & components()
	{
		static std::vector<C> data{C{}};

		return data;
	}

	virtual ~Component() {}
};

template<typename C>
std::vector<C> & components()
{
	return Component<C>::components();
}

template<typename C>
void buildMask(std::bitset<ecs::MAX_COMPONENTS> &bitmask)
{
	bitmask.set(C::bit());
}

template<typename C1, typename C2, typename... CC>
void buildMask(std::bitset<ecs::MAX_COMPONENTS> &bitmask)
{
	bitmask.set(C1::bit());

	buildMask<C2, CC...>(bitmask);
}

template<typename C>
std::bitset<ecs::MAX_COMPONENTS> mask()
{
	std::bitset<ecs::MAX_COMPONENTS> bitmask{};

	bitmask.set(C::bit());

	return bitmask;
}

template<typename C1, typename C2, typename... CC>
std::bitset<ecs::MAX_COMPONENTS> mask()
{
	std::bitset<ecs::MAX_COMPONENTS> bitmask{};

	buildMask<C1, C2, CC...>(bitmask);

	return bitmask;
}

}; // ecs

#endif