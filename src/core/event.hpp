#pragma once

#include <core/util/str.hpp>

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include <iostream>


namespace event
{

template<class T>
class Event
{
public:
	static size_t & listenerCount()
	{
		static size_t count = 0;

		return count;
	}

	static std::vector<std::function<void(const T &)>> & listeners()
	{
		static std::vector<std::function<void(const T &)>> container{};

		return container;
	}

	static size_t subscribe(std::function<void(const T &)> &&callback)
	{
		auto &container = Event<T>::listeners();
		auto &count = Event<T>::listenerCount();

		if (container.size() == count)
		{
			container.push_back(std::move(callback));
			count++;

			return container.size();
		}
		else
		{
			for (size_t i = 0; i < container.size(); i++)
			{
				if ( ! container[i])
				{
					container[i] = std::move(callback);
					count++;

					return i + 1;
				}
			}

			// this should not happen...
			container.push_back(std::move(callback));
			count++;

			return container.size();
		}
	}

	static void unsubscribe(size_t callbackId)
	{
		if ( ! callbackId)
		{
			return;
		}

		auto &container = Event<T>::listeners();
		auto &count = Event<T>::listenerCount();

		if (container.size() < callbackId)
		{
			return;
		}

		container[callbackId - 1] = nullptr;
		count--;
	}

	virtual ~Event() {}
};

template<class T>
class Listener
{
public:
	std::vector<size_t> indices{};

	size_t attach(std::function<void(const T &)> &&callback)
	{
		indices.push_back(Event<T>::subscribe(std::move(callback)));
		return indices.back();

		/*
		auto &container = Event<T>::listeners();
		auto &count = Event<T>::listenerCount();

		if (container.size() == count)
		{
			container.push_back(std::move(callback));
			count++;

			indices.push_back(container.size());
			return indices.back();
		}
		else
		{
			for (size_t i = 0; i < container.size(); i++)
			{
				if ( ! container[i])
				{
					container[i] = std::move(callback);
					count++;

					indices.push_back(i + 1);
					return indices.back();
				}
			}

			// this should not happen...
			container.push_back(std::move(callback));
			count++;

			indices.push_back(container.size());
			return indices.back();
		}
		*/
	}

	void detach(size_t callbackId)
	{
		if (std::find(indices.begin(), indices.end(), callbackId) == indices.end())
		{
			return;
		}

		Event<T>::unsubscribe(callbackId);

		indices.erase(std::remove(indices.begin(), indices.end(), callbackId), indices.end());

		/*
		if ( ! callbackId)
		{
			return;
		}

		auto &container = Event<T>::listeners();
		auto &count = Event<T>::listenerCount();

		container[callbackId - 1] = nullptr;
		indices.erase(std::remove(indices.begin(), indices.end(), callbackId), indices.end());

		count--;
		*/
	}

	void reset()
	{

		for (const auto idx : indices)
		{
			Event<T>::unsubscribe(idx);
		}

		indices.clear();
	}

	~Listener()
	{
		reset();
	}
};

template<typename T>
static std::vector<std::function<void(const T &)>> & listeners()
{
	return Event<T>::listeners();
}

template<typename T>
static size_t & listenerCount()
{
	return Event<T>::listenerCount();
}

template<typename T>
static void emit(const T &event)
{
	for (const auto &listener : Event<T>::listeners())
	{
		if (listener)
		{
			listener(event);
		}
	}
}

} // event