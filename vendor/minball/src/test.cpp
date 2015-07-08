#include <minball/minball.hpp>
#include <iostream>

using namespace std;

int main(int argn,char **argv)
{
	Minball minball{};

	minball.setPoints({
		 1.f,  1.f, -1.f,
		-1.f,  1.f, -1.f,
		 1.f, -1.f, -1.f,
		-1.f, -1.f, -1.f,
		 1.f,  1.f,  1.f,
		-1.f,  1.f,  1.f,
		 1.f, -1.f,  1.f,
		-0.f, -0.f,  0.f
	});

	minball.setPoint(7, -1.f, -1.f, 1.f);

	cout << "radius=" << minball.radius() << endl;

	auto center = minball.center();
	cout << "center=" << center[0] << ", " << center[1] << ", " << center[2] << endl;

	return 0;
}
