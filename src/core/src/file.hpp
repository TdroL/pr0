#ifndef SRC_FILE_HPP
#define SRC_FILE_HPP

#include "../src.hpp"
#include <string>

namespace src
{

namespace file
{

class Stream : public src::Stream
{
public:
	std::string fileName{};

	Stream();
	Stream(std::string &&fileName);
	void use();
	std::string name();
};

std::unique_ptr<src::Stream> stream(const std::string &fileName);
std::unique_ptr<src::Stream> stream(std::string &&fileName);

} // file

} // src

#endif