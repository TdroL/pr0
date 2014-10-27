#ifndef MPRINT_HPP
#define MPRINT_HPP

#include <iostream>
#include <string>
#include <vector>

namespace mprint
{

static std::string list_sep = ", ";
static std::string list_beg = "[";
static std::string list_end = "]";

namespace details
{

class Mprint
{
public:
	std::string sep;
	std::string eol;

	Mprint(const std::string &sep, const std::string &eol)
	 : sep{sep}, eol{eol}
	{}

	~Mprint()
	{
		std::cout << eol;
	}

	template<typename T>
	Mprint & operator+=(const T &t)
	{
		std::cout << t;
		return *this;
	}

	template<typename T>
	Mprint & operator,(const T &t)
	{
		std::cout << sep;
		return this->operator+=(t);
	}

	template<typename T>
	Mprint & operator+=(const std::vector<T> &vec)
	{
		std::cout << list_beg;

		if (vec.size() > 0)
		{
			auto it = std::begin(vec);
			std::cout << *it;

			while (++it < std::end(vec))
			{
				std::cout << list_sep << *it;
			}
		}

		std::cout << list_end;

		return *this;
	}

	template<typename T>
	Mprint & operator,(const std::vector<T> &vec)
	{
		std::cout << sep;
		return this->operator+=(vec);
	}
};

} // details

} // mprint

#define print mprint::details::Mprint{" ", "\n"} +=
#define puts mprint::details::Mprint{" ", ""} +=
#define echo mprint::details::Mprint{"", "\n"} +=
#define echon mprint::details::Mprint{"", ""} +=

#define var(expr) mprint::details::Mprint{" ", "\n"} += #expr, "=", (expr)

#endif