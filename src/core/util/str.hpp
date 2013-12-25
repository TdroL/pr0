#ifndef UTIL_STR_HPP
#define UTIL_STR_HPP

#include <string>
#include <algorithm>

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

}

}

#endif