#ifndef UTIL_STR_HPP
#define UTIL_STR_HPP

#include <string>

namespace util
{

namespace str
{

void ltrim(std::string &s);

void rtrim(std::string &s);

void trim(std::string &s);

std::string demangle(const char* name);

}

}

#endif