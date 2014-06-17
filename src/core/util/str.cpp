#include "str.hpp"

#include <algorithm>
#include <cstdlib>
#include <memory>

#ifdef __GNUG__
	#include <cxxabi.h>
#endif

namespace util
{

namespace str
{

void ltrim(std::string &s) {
	s.erase(std::begin(s), std::find_if(std::begin(s), std::end(s), std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), std::end(s));
}

void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

#ifdef __GNUG__

std::string demangle(const char* name)
{

	int status = -4; // some arbitrary value to eliminate the compiler warning

	// enable c++11 by passing the flag -std=c++11 to g++
	std::unique_ptr<char, void(*)(void*)> res{
		abi::__cxa_demangle(name, NULL, NULL, &status),
		std::free
	};

	return (status==0) ? res.get() : name ;
}

#else

// does nothing if not g++
std::string demangle(const char* name) {
	return name;
}

#endif

}

}