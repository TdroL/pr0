#ifndef NGN_HPP
#define NGN_HPP

#include <limits>

#include "util/initq.hpp"

namespace ngn
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

} // ngn

#endif