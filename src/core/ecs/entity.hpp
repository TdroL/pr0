#ifndef ECS_ENTITY_HPP
#define ECS_ENTITY_HPP

#include "../ecs.hpp"
#include "component.hpp"

#include "../util/str.hpp"
#include "../event.hpp"

#include <vector>
#include <bitset>
#include <algorithm>
#include <typeinfo>

namespace ecs
{

typedef size_t Entity;

extern std::vector<std::bitset<ecs::MAX_COMPONENTS>> entityComponentMasks;

/* #Events */

struct EntityCreateEvent : public event::Event<EntityCreateEvent>
{
	const Entity &entity;
	explicit EntityCreateEvent(const Entity &entity) : entity(entity) {}
};

struct EntityDestroyEvent : public event::Event<EntityDestroyEvent>
{
	const Entity &entity;
	explicit EntityDestroyEvent(const Entity &entity) : entity(entity) {}
};

template<typename C>
struct EntityEnableEvent : public event::Event<EntityEnableEvent<C>>
{
	const Entity &entity;
	explicit EntityEnableEvent(const Entity &entity) : entity(entity) {}
};

template<typename C>
struct EntityDisableEvent : public event::Event<EntityDisableEvent<C>>
{
	const Entity &entity;
	explicit EntityDisableEvent(const Entity &entity) : entity(entity) {}
};

/* /Events */

/* #Iterators */

class EntityIteratorWrapper
{
public:
	const std::bitset<MAX_COMPONENTS> bitmask;

	class Iterator
	{
	public:
		const std::bitset<MAX_COMPONENTS> bitmask;
		Entity entity;

		Iterator(const std::bitset<MAX_COMPONENTS> &bitmask, const Entity &entity);

		Entity & operator*();
		Iterator& operator++();
		bool operator!=(const Iterator &rhs);
	};

	explicit EntityIteratorWrapper(const std::bitset<MAX_COMPONENTS> &bitmask);

	Iterator begin();
	Iterator end();
};

/* /Iterators */

/* #Core */

Entity create();
void destroy(Entity &entity);

template<typename C>
void enable(const Entity &entity)
{
	entityComponentMasks[entity].set(C::bit());

	auto &data = ecs::components<C>();

	if (data.size() <= entity)
	{
		data.resize(std::max(static_cast<size_t>(entity), static_cast<size_t>(32)));
	}

	event::emit(EntityEnableEvent<C>{entity});
}

template<typename C1, typename C2, typename... CC>
void enable(const Entity &entity)
{
	enable<C1>(entity);
	enable<C2, CC...>(entity);
}

template<typename C>
void disable(const Entity &entity)
{
	entityComponentMasks[entity].reset(C::bit());

	event::emit(EntityDisableEvent<C>{entity});
}

template<typename C1, typename C2, typename... CC>
void disable(const Entity &entity)
{
	disable<C1>(entity);
	disable<C2, CC...>(entity);
}

bool has(const Entity &entity, const std::bitset<MAX_COMPONENTS> &bitmask);

template<typename C>
bool has(const Entity &entity)
{
	return entityComponentMasks[entity].test(C::bit());
}

template<typename C1, typename C2, typename... CC>
bool has(const Entity &entity)
{
	const auto bitmask = mask<C1, C2, CC...>();

	return has(entity, bitmask);
}

template<typename C, typename... CC>
EntityIteratorWrapper findWith()
{
	return EntityIteratorWrapper{mask<C, CC...>()};
}

EntityIteratorWrapper findWith(const std::bitset<MAX_COMPONENTS> &bitmask);

template <typename C>
C & get(const Entity &entity)
{
	auto &components = ecs::components<C>();

	if (components.size() < entity)
	{
		throw std::string{"ecs::get<" + util::str::demangle(typeid(C).name()) + ">(" + std::to_string(entity) + ") - invalid entity"};
	}

	return components[entity];
}

/* /Core */

}; // ecs

#endif