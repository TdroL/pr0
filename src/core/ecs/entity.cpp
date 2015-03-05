#include "entity.hpp"
#include "../event.hpp"

#include <set>

namespace ecs
{

using namespace std;

vector<bitset<ecs::MAX_COMPONENTS>> entityComponentMasks{};

namespace
{
	vector<unsigned int> freeIds{};
}

/* #Iterators */

EntityIteratorWrapper::Iterator::Iterator(const bitset<MAX_COMPONENTS> &bitmask, const Entity &entity)
	: bitmask{bitmask}, entity{entity}
{}

Entity & EntityIteratorWrapper::Iterator::operator*()
{
	return entity;
}

EntityIteratorWrapper::Iterator & EntityIteratorWrapper::Iterator::operator++()
{
	while(entity < entityComponentMasks.size())
	{
		entity++;

		if (has(entity, bitmask))
		{
			return *this;
		}
	}

	entity = entityComponentMasks.size();

	return *this;
}

bool EntityIteratorWrapper::Iterator::operator!=(const EntityIteratorWrapper::Iterator &rhs)
{
	return entity != rhs.entity;
}

EntityIteratorWrapper::EntityIteratorWrapper(const bitset<MAX_COMPONENTS> &bitmask)
	: bitmask{bitmask}
{}

EntityIteratorWrapper::Iterator EntityIteratorWrapper::begin()
{
	return ++EntityIteratorWrapper::Iterator{bitmask, 0};
}

EntityIteratorWrapper::Iterator EntityIteratorWrapper::end()
{
	return EntityIteratorWrapper::Iterator{bitmask, entityComponentMasks.size()};
}

/* /Iterators */

/* #Core */

Entity create()
{
	static unsigned int globalId = 1;

	unsigned int id = 0;

	if ( ! freeIds.empty())
	{
		id = freeIds.back();
		freeIds.pop_back();
	}

	if (id == 0)
	{
		id = globalId;
		globalId++;
	}

	if (entityComponentMasks.size() <= id)
	{
		entityComponentMasks.resize(static_cast<size_t>((id + 1) * 2));
	}

	Entity entity{id};

	event::emit(EntityCreateEvent{entity});

	return entity;
}

void destroy(Entity &entity)
{
	event::emit(EntityDestroyEvent{entity});

	entityComponentMasks[entity].reset();

	freeIds.push_back(entity);
}

bool has(const Entity &entity, const bitset<MAX_COMPONENTS> &bitmask)
{
	return (entityComponentMasks[entity] & bitmask) == bitmask;
}

EntityIteratorWrapper findWith(const bitset<MAX_COMPONENTS> &bitmask)
{
	return EntityIteratorWrapper{bitmask};
}

/* /Core */

} // ecs