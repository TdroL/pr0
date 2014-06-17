#ifndef EVENT_HPP
#define EVENT_HPP

#include <vector>

namespace event
{

template<class T>
class Event
{
public:
	static std::vector<void(*)(const T &)> & listeners()
	{
		static std::vector<void(*)(const T &)> container{};

		return container;
	}

	virtual ~Event() {}
};

template<typename T>
static std::vector<void(*)(const T &)> & listeners()
{
	return Event<T>::listeners();
}

template<typename T>
static void emit(const T &event)
{
	for (const auto &listener : listeners<T>())
	{
		listener(event);
	}
}

template<typename T>
static void subscribe(void (*reciver)(const T &))
{
	listeners<T>().push_back(reciver);
}

} // event

#endif