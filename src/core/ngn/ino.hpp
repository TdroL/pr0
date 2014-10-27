#ifndef NGN_INO_HPP
#define NGN_INO_HPP

#include <map>
#include <string>

namespace ngn
{

namespace ino
{

extern std::map<std::string, std::string> args;

void init(int argc, char const* argv[]);

bool has(const std::string &param);

std::string get(const std::string &param);

template<typename T>
T get(const std::string &param, T defaultValut);

template<>
int get(const std::string &param, int defaultValue);

template<>
std::string get(const std::string &param, std::string defaultValue);

} // ino

} // ngn

#endif