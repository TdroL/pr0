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

using namespace std;

void ltrim(string &s) {
	s.erase(begin(s), find_if(begin(s), end(s), not1(ptr_fun<int, int>(isspace))));
}

void rtrim(string &s) {
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), end(s));
}

void trim(string &s) {
	ltrim(s);
	rtrim(s);
}

#ifdef __GNUG__

string demangle(const char* name) {

	int status = -1;

	unique_ptr<char, void(*)(void*)> res{abi::__cxa_demangle(name, nullptr, nullptr, &status), free};

	return (status == 0) ? res.get() : name ;
}

#else

// does nothing if not g++
string demangle(const char* name) {
	return name;
}

#endif

} // str

} // util