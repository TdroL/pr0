#include "prof.hpp"
#include <iostream>
#include <algorithm>

namespace rn
{

using namespace std;

vector<Prof *> Prof::collection{};

void Prof::reloadSoftAll()
{
	for (Prof *prof : Prof::collection)
	{
		try
		{
			prof->reloadSoft();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

void Prof::reloadAll()
{
	for (Prof *prof : Prof::collection)
	{
		try
		{
			prof->reload();
		}
		catch (const string &e)
		{
			clog << endl << e << endl;
		}
	}
}

void Prof::swapAll()
{
	for (Prof *prof : Prof::collection)
	{
		prof->swap();
	}
}

Prof::Prof()
{
	Prof::collection.push_back(this);

}

Prof::Prof(string &&profName)
	: Prof{}
{
	this->profName = move(profName);
}

Prof::~Prof()
{
	reset();

	// Prof::collection.remove(this);
	Prof::collection.erase(remove(begin(Prof::collection), end(Prof::collection), this), end(Prof::collection));
}

void Prof::init()
{
	reloadSoft();
}

void Prof::reset()
{
	RN_CHECK(glDeleteQueries(2, queries[0]));
	RN_CHECK(glDeleteQueries(2, queries[1]));

	times[0] = 0;
	times[1] = 0;

	front = 0;
	back = 0;
}

void Prof::reloadSoft()
{
	reset();

	RN_CHECK(glGenQueries(2, queries[0]));
	RN_CHECK(glGenQueries(2, queries[1]));
}

void Prof::reload()
{
	reloadSoft();
}

void Prof::swap()
{
	times[back] = 0;

	back = front;
	front = 1 - front;
}

void Prof::start()
{
	RN_CHECK_PARAM(glQueryCounter(queries[front][0], GL_TIMESTAMP), profName << " id=" << queries[front][0]);
}

void Prof::stop()
{
	RN_CHECK_PARAM(glQueryCounter(queries[front][1], GL_TIMESTAMP), profName << " id=" << queries[front][1]);

	if (front != back)
	{
		GLuint64 t1;
		GLuint64 t2;

		RN_CHECK(glGetQueryObjectui64v(queries[back][0], GL_QUERY_RESULT, &t1));
		RN_CHECK(glGetQueryObjectui64v(queries[back][1], GL_QUERY_RESULT, &t2));

		times[back] = t2 - t1;
	}
}

GLuint64 Prof::ns()
{
	return times[back];
}

double Prof::ms()
{
	return times[back] / 1000000.0;
}

} // rn