#include "args.hpp"

#include <map>
#include <vector>
#include <string>
#include <iostream>


namespace args
{

using namespace std;

map<char, int> args;
vector<string> files;
int argc;
char **argv;

void parse(int argc, char *argv[])
{
	args::argc = argc;
	args::argv = argv;

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			const char *ch = &argv[i][1];
			while (*ch)
			{
				args[*ch] = i;
				ch++;
			}
		}
		else
		{
			files.push_back(argv[i]);
		}
	}
}

void debug()
{
	cout << "args:" << endl;

	for (auto &item : args)
	{
		cout << item.first << "=" << item.second << endl;
	}

	cout << "files:" << endl;

	for (auto &file : files)
	{
		cout << file << endl;
	}
}

bool has(char ch)
{
	return args.find(ch) != end(args);
}

bool hasAny(char *ch)
{
	while (*ch)
	{
		if (has(*ch))
		{
			return true;
		}

		ch++;
	}

	return false;
}

vector<char *> get(char ch)
{
	vector<char *> values{};

	auto it = args.find(ch);

	if (it != end(args))
	{
		for (int i = it->second + 1; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				break;
			}

			values.push_back(argv[i]);
		}
	}

	return values;
}

char * last()
{
	if (argc == 1)
	{
		return nullptr;
	}

	return argv[argc - 1];
}

}