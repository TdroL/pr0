#ifndef SYS_HPP
#define SYS_HPP

namespace sys
{

extern double ct;
extern double dt;

void init();

void deinit();

void update();

void startLoop();

void endLoop();

// void args(int argc, char const* argv[]);

// void run();

double time();

} // sys

#endif