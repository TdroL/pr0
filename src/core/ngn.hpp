#ifndef NGN_HPP
#define NGN_HPP

#include <limits>

#include "util/initq.hpp"

namespace ngn
{

extern double ct;
extern double dt;

util::InitQ & initQ();

void init();

void deinit();

void update();

void startLoop();

void endLoop();

double time();

} // ngn

#endif