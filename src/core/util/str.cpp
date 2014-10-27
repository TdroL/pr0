#include "str.hpp"

#include <algorithm>
#include <cstdlib>
#include <cctype>
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

bool isInt(const string &s)
{
	return !s.empty() && s.find_first_not_of("0123456789") == string::npos;
}

bool isNumeric(const string &s)
{
	int dots = 0;
	return !s.empty() && end(s) == find_if(begin(s), end(s), [&dots](char c)
		{
			if (c == '.') {
				dots++;

				if (dots > 1) {
					return true;
				}

				return false;
			}

			return !isdigit(c);
		});
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