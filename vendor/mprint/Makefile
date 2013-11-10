CC := $(CXX)
RM ?= rm -f

CXXFLAGS = -Wall -Winline -Wextra -Wfatal-errors -std=c++11 -m64 -march=native -g -O0 -Weffc++ -fmax-errors=10


test: tests/mprint_test.cpp
	$(CXX) $(CXXFLAGS) -DTEST $< -o $@
	./test