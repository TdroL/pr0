#include <pch.hpp>

#include "prof.hpp"
#include "ext.hpp"

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
	RN_CHECK(glDeleteQueries(maxQueries * 2, &queries[0][0]));
	// RN_CHECK(glDeleteQueries(2, queries[1]));

	// times[0] = 0;
	// times[1] = 0;

	dt = 0;
	df = 0;

	front = 0;
	back = 0;
}

void Prof::reloadSoft()
{
	double timer = ngn::time();

	reset();

	// RN_CHECK(glGenQueries(2, queries[0]));
	// RN_CHECK(glGenQueries(2, queries[1]));

	RN_CHECK(glCreateQueries(GL_TIMESTAMP, maxQueries * 2, &queries[0][0]));

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
	if ( ! active)
	{
		dt = 0;
		df = 0;
	}

	active = false;
	/*
	if (back != -1)
	{
		times[back] = 0;
	}

	if (front != -1)
	{
		back = front;
		front = 1 - front;
	}
	*/
}

void Prof::start()
{
	if ( ! queries[front][START])
	{
		return;
	}

	active = true;
	RN_CHECK_PARAM(glQueryCounter(queries[front][START], GL_TIMESTAMP), profName << " id=" << queries[front][START]);

	// if (profName == "App::profSetupLights")
	// {
	// 	cout << "start front=" << front << " back=" << back << " queries[front][START]=" << queries[front][START] << endl;
	// }

	/*
	if (front == -1 && queries[0][START])
	{
		front = 0;
	}

	if (front != -1 && queries[front][START])
	{
		// RN_CHECK_PARAM(glQueryCounter(queries[front][START], GL_TIMESTAMP), profName << " id=" << queries[front][START]);
	}
	*/
}

void Prof::stop()
{
	if ( ! queries[front][STOP])
	{
		return;
	}

	active = true;
	RN_CHECK_PARAM(glQueryCounter(queries[front][STOP], GL_TIMESTAMP), profName << " id=" << queries[front][START]);

	front = (front + 1) % maxQueries;

	GLuint available;
	GLuint64 t1 = 0;
	GLuint64 t2 = 0;
	int frames = 0;
	int query = front - 1;

	for (size_t i = 0; i < maxQueries; i++)
	{
		if (query < 0)
		{
			query = maxQueries - 1;
		}

		RN_CHECK(glGetQueryObjectuiv(queries[query][STOP], GL_QUERY_RESULT_AVAILABLE, &available));

		if (available)
		{
			RN_CHECK(glGetQueryObjectui64v(queries[query][START], GL_QUERY_RESULT, &t1));
			RN_CHECK(glGetQueryObjectui64v(queries[query][STOP], GL_QUERY_RESULT, &t2));

			dt = t2 - t1;
			df = frames;
			back = (query + 1) % maxQueries;
			break;
		}
		else if (query == back)
		{
			break;
		}
		else
		{
			frames++;
			query--;
		}
	}

	/*
	frames++;
	query--;
	if (query < 0)
	{
		query = maxQueries - 1;
	}

	RN_CHECK(glGetQueryObjectuiv(queries[query][STOP], GL_QUERY_RESULT_AVAILABLE, &available));

	if (available)
	{
		RN_CHECK(glGetQueryObjectui64v(queries[query][START], GL_QUERY_RESULT, &t1));
		RN_CHECK(glGetQueryObjectui64v(queries[query][STOP], GL_QUERY_RESULT, &t2));

		dt = t2 - t1;
		df = frames;
		back = (query + 1) % maxQueries;
	}
	else if (query != back)
	{
		frames++;
		query--;
		if (query < 0)
		{
			query = maxQueries - 1;
		}

		RN_CHECK(glGetQueryObjectuiv(queries[query][STOP], GL_QUERY_RESULT_AVAILABLE, &available));

		if (available)
		{
			RN_CHECK(glGetQueryObjectui64v(queries[query][START], GL_QUERY_RESULT, &t1));
			RN_CHECK(glGetQueryObjectui64v(queries[query][STOP], GL_QUERY_RESULT, &t2));

			dt = t2 - t1;
			df = frames;
			back = (query + 1) % maxQueries;
		}
		else
		{
			frames++;
			query--;
			if (query < 0)
			{
				query = maxQueries - 1;
			}

			RN_CHECK(glGetQueryObjectuiv(queries[query][STOP], GL_QUERY_RESULT_AVAILABLE, &available));

			if (available)
			{
				RN_CHECK(glGetQueryObjectui64v(queries[query][START], GL_QUERY_RESULT, &t1));
				RN_CHECK(glGetQueryObjectui64v(queries[query][STOP], GL_QUERY_RESULT, &t2));

				dt = t2 - t1;
				df = frames;
				back = (query + 1) % maxQueries;
			}
			else
			{
				frames++;
				query--;
				if (query < 0)
				{
					query = maxQueries - 1;
				}

				RN_CHECK(glGetQueryObjectuiv(queries[query][STOP], GL_QUERY_RESULT_AVAILABLE, &available));

				if (available)
				{
					RN_CHECK(glGetQueryObjectui64v(queries[query][START], GL_QUERY_RESULT, &t1));
					RN_CHECK(glGetQueryObjectui64v(queries[query][STOP], GL_QUERY_RESULT, &t2));

					dt = t2 - t1;
					df = frames;
					back = (query + 1) % maxQueries;
				}
				else
				{
					frames++;
					query--;
					if (query < 0)
					{
						query = maxQueries - 1;
					}

					RN_CHECK(glGetQueryObjectuiv(queries[query][STOP], GL_QUERY_RESULT_AVAILABLE, &available));

					if (available)
					{
						RN_CHECK(glGetQueryObjectui64v(queries[query][START], GL_QUERY_RESULT, &t1));
						RN_CHECK(glGetQueryObjectui64v(queries[query][STOP], GL_QUERY_RESULT, &t2));

						dt = t2 - t1;
						df = frames;
						back = (query + 1) % maxQueries;
					}
					else
					{
						cout << profName << " too deep!" << endl;
					}
				}
			}
		}
	}
	*/

	// if (profName == "App::profSetupLights")
	// {
	// 	cout << "stop front=" << front << " back=" << back << " queries[front][STOP]=" << queries[front][STOP] << " queries[back][START]=" << queries[back][START] << " queries[back][STOP]=" << queries[back][STOP] << endl;
	// }

	/*
	if (front != -1 && queries[front][STOP])
	{
		RN_CHECK_PARAM(glQueryCounter(queries[front][STOP], GL_TIMESTAMP), profName << " id=" << queries[front][STOP]);
	}

	if (back != -1)
	{
		GLuint available;
		GLuint64 t1 = 0;
		GLuint64 t2 = 0;

		if (queries[back][START])
		{
			RN_CHECK(glGetQueryObjectuiv(queries[back][START], GL_QUERY_RESULT_AVAILABLE, &available));

			// cout << profName << " START available=" << available << endl;

			if (available == GL_TRUE)
			{
				RN_CHECK(glGetQueryObjectui64v(queries[back][START], GL_QUERY_RESULT, &t2));
			}
		}

		if (queries[back][STOP])
		{
			RN_CHECK(glGetQueryObjectuiv(queries[back][STOP], GL_QUERY_RESULT_AVAILABLE, &available));

			// cout << profName << " STOP available=" << available << endl;

			if (available == GL_TRUE)
			{
				RN_CHECK(glGetQueryObjectui64v(queries[back][STOP], GL_QUERY_RESULT, &t2));
			}
			else
			{
				t2 = t1;
			}
		}

		times[back] = t2 - t1;
	}
	*/
}

int Prof::latency()
{
	return df;
}

GLuint64 Prof::ns()
{
	return dt;
}

double Prof::ms()
{
	return ns() / 1000000.0;
}

namespace
{
	const util::InitQAttacher attach(rn::initQ(), []
	{
		/*
		if ( ! rn::ext::ARB_direct_state_access)
		{
			throw string{"rn::Prof initQ - rn::Prof requires GL_ARB_direct_state_access"};
		}
		*/
	});
}

} // rn