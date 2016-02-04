#include <minball/minball.hpp>
#include <Seb.h>

class MinballImpl {
public:
	typedef Seb::Point<Minball::ImplType> Point;
	typedef std::vector<Point> Container;

	Container points;
	Seb::Smallest_enclosing_ball<Minball::ImplType, Point, Container> mb;

	explicit MinballImpl(size_t capacity);

	void setPoints(const std::vector<Minball::DataType> &newPoints);
	void setPoint(size_t index, Minball::DataType x, Minball::DataType y, Minball::DataType z);
	Minball::DataType radius();
	std::array<Minball::DataType, Minball::dimensions> center();
};

// Minball

Minball::Minball(size_t capacity)
	: pimpl{new MinballImpl(capacity)}
{}

Minball::Minball(Minball &&rhs)
	: pimpl{std::move(rhs.pimpl)}
{
	rhs.pimpl.reset();
}

Minball::~Minball()
{
	reset();
}

Minball & Minball::operator=(Minball &&rhs)
{
	pimpl = std::move(rhs.pimpl);
	rhs.pimpl.reset();
}

void Minball::setPoints(const std::vector<Minball::DataType> &points)
{
	pimpl->setPoints(points);
}

void Minball::setPoint(size_t index, Minball::DataType x, Minball::DataType y, Minball::DataType z)
{
	pimpl->setPoint(index, x, y, z);
}

Minball::DataType Minball::radius()
{
	return pimpl->radius();
}

std::array<Minball::DataType, Minball::dimensions> Minball::center()
{
	return pimpl->center();
}

void Minball::reset()
{
	pimpl.reset();
}

// MinballImpl

MinballImpl::MinballImpl(size_t capacity)
	: points(capacity, Point{Minball::dimensions}), mb(Minball::dimensions, points, false)
{}

void MinballImpl::setPoints(const std::vector<Minball::DataType> &newPoints)
{
	mb.invalidate();

	for (size_t i = 0; i < points.size(); i++)
	{
		for (int j = 0; j < Minball::dimensions; j++)
		{
			size_t index = i * Minball::dimensions + j;

			if (index >= newPoints.size())
			{
				return;
			}

			points[i][j] = newPoints[index];
		}
	}
}

void MinballImpl::setPoint(size_t index, Minball::DataType x, Minball::DataType y, Minball::DataType z)
{
	mb.invalidate();

	points[index][0] = x;
	points[index][1] = y;
	points[index][2] = z;
}

Minball::DataType MinballImpl::radius()
{
	return mb.radius();
}

std::array<Minball::DataType, Minball::dimensions> MinballImpl::center()
{
	std::array<Minball::DataType, Minball::dimensions> center;

	auto it = mb.center_begin();

	for (int i = 0; i < Minball::dimensions; i++)
	{
		center[i] = *(it + i);
	}

	return center;
}