#ifndef ARGS_HPP
#define ARGS_HPP

#include <vector>

namespace args
{

void parse(int argc, char *argv[]);

void debug();

bool has(char ch);

bool hasAny(char *ch);

std::vector<char *> get(char ch);

char * last();

}

#endif