#ifndef SYS_HPP
#define SYS_HPP

#include <limits>

#include "util/initq.hpp"

namespace sys
{

extern util::InitQ initQ;

extern double ct;
extern double dt;

void init();

void deinit();

void update();

void startLoop();

void endLoop();

double time();

} // sys

#endif