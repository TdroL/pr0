#include <pch.hpp>

#include "prof.hpp"

#include "../ngn.hpp"

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

	front = -1;
	back = -1;
}

void Prof::reloadSoft()
{
	double timer = ngn::time();

	reset();

	RN_CHECK(glGenQueries(2, queries[0]));
	RN_CHECK(glGenQueries(2, queries[1]));

	UTIL_DEBUG
	{
		clog << fixed;
		clog << "  [Prof \"" << profName << "\":" << (ngn::time() - timer) << "s]" << endl;
		clog.unsetf(ios::floatfield);
	}
}

void Prof::reload()
{
	reloadSoft();
}

void Prof::swap()
{
	times[back] = 0;

	if (front != -1)
	{
		back = front;
		front = 1 - front;
	}
}

void Prof::start()
{
	if (queries[front][START])
	{
		if (front == -1)
		{
			front = 0;
		}

		RN_CHECK_PARAM(glQueryCounter(queries[front][START], GL_TIMESTAMP), profName << " id=" << queries[front][START]);
	}
}

void Prof::stop()
{
	if (front != -1 && queries[front][STOP])
	{
		RN_CHECK_PARAM(glQueryCounter(queries[front][STOP], GL_TIMESTAMP), profName << " id=" << queries[front][STOP]);
	}

	if (back != -1)
	{
		GLuint64 t1 = 0;
		GLuint64 t2 = 0;

		if (queries[back][START])
		{
			RN_CHECK(glGetQueryObjectui64v(queries[back][START], GL_QUERY_RESULT, &t1));
		}

		if (queries[back][STOP])
		{
			RN_CHECK(glGetQueryObjectui64v(queries[back][STOP], GL_QUERY_RESULT, &t2));
		}

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