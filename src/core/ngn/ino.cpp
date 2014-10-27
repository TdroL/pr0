#include "ino.hpp"
#include "../util/str.hpp"

namespace ngn
{

namespace ino
{

using namespace std;

map<string, string> args{};

void init(int argc, char const* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		string name = argv[i];
		string value = argv[i];

		auto separator = name.find('=');

		if (separator != string::npos) {
			name = name.substr(0, separator);
			value = value.substr(separator + 1);
		}

		args.emplace(make_pair(name, value));
	}
}

bool has(const string &param)
{
	return args.find(param) != end(args);
}

string get(const string &param)
{
	return get<string>(param, string{});
}

template<>
int get(const string &param, int defaultValue)
{
	const auto it = args.find(param);

	if (it != end(args) && util::str::isInt(it->second))
	{
		return stoi(it->second);
	}

	return defaultValue;
}

template<>
string get(const string &param, string defaultValue)
{
	const auto it = args.find(param);

	if (it != end(args))
	{
		return it->second;
	}

	return defaultValue;
}

} // ino

} // ngn