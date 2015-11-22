#ifndef MINBALL_HPP
#define MINBALL_HPP

#include <array>
#include <vector>
#include <memory>

class MinballImpl;

class Minball {
public:
	typedef float DataType;
	typedef double ImplType;

	static constexpr int dimensions = 3;

	explicit Minball(int capacity = 8);
	Minball(Minball &&rhs);
	Minball(const Minball &lhs) = delete;

	~Minball();

	Minball & operator=(Minball &&rhs);
	Minball & operator=(const Minball &lhs) = delete;

	void setPoints(const std::vector<DataType> &points);
	void setPoint(size_t index, DataType x, DataType y, DataType z);
	DataType radius();
	std::array<DataType, dimensions> center();
private:
	std::unique_ptr<MinballImpl> pimpl;

	void reset();
};

#endif