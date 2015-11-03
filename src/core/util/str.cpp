#include "str.hpp"

#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <sstream>
#include <memory>

#ifdef __GNUG__
	#include <cxxabi.h>
#endif

namespace util
{

namespace str
{

using namespace std;

void lowercase(string &s)
{
	transform(begin(s), end(s), begin(s), ::tolower);
}

void uppercase(string &s)
{
	transform(begin(s), end(s), begin(s), ::toupper);
}

void ltrim(string &s)
{
	s.erase(begin(s), find_if(begin(s), end(s), not1(ptr_fun<int, int>(isspace))));
}

void rtrim(string &s)
{
	s.erase(find_if(rbegin(s), rend(s), not1(ptr_fun<int, int>(isspace))).base(), end(s));
}

void trim(string &s)
{
	ltrim(s);
	rtrim(s);
}

bool isInt(const string &s)
{
	return ! s.empty() && s.find_first_not_of("0123456789") == string::npos;
}

bool isNumeric(const string &s)
{
	int dots = 0;
	return ! s.empty() && end(s) == find_if(begin(s), end(s), [&dots](char c)
	{
		if (c == '.')
		{
			dots++;

			return (dots > 1);
		}

		return ! isdigit(c);
	});
}

string prependLineNumbers(const std::string &s)
{
	istringstream in{s};
	string line{};
	string out{};
	string zeroes{};

	size_t newlineCount = count(begin(s), end(s), '\n');
	size_t zeroesCount = ceil(log10(newlineCount));
	zeroes.append('0', zeroesCount);
	out.reserve(s.length() + newlineCount * (zeroesCount + 2 + 1));

	for (size_t i = 1; getline(in, line); i++)
	{
		string lineNumber{zeroes + to_string(i)};

		out += lineNumber.substr(lineNumber.size() - zeroesCount) + ": " + line + "\n";
	}

	return out;
}

#ifdef __GNUG__

string demangle(const char* name)
{
	int status = -1;

	unique_ptr<char, void(*)(void*)> res{abi::__cxa_demangle(name, nullptr, nullptr, &status), free};

	return (status == 0) ? res.get() : name ;
}

#else

string demangle(const char* name)
{
	return name;
}

#endif

} // str

} // util