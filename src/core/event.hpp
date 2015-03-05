#ifndef EVENT_HPP
#define EVENT_HPP

#include <vector>
#include <functional>
#include <memory>

namespace event
{

template<class T>
class Event
{
public:
	static std::vector<std::function<void(const T &)>> & listeners()
	{
		static std::vector<std::function<void(const T &)>> container{};

		return container;
	}

	virtual ~Event() {}
};

template<typename T>
static std::vector<std::function<void(const T &)>> & listeners()
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
static void subscribe(std::function<void(const T &)> &&reciver)
{
	listeners<T>().push_back(std::move(reciver));
}

} // event

#endif