#include "file.hpp"
#include "../ngn/fs.hpp"

#include <vector>
#include <iostream>

namespace src
{

namespace file
{

using namespace std;

namespace fs = ngn::fs;

Stream::Stream()
	: fileName{""}
{}

Stream::Stream(std::string &&fileName)
	: fileName{move(fileName)}
{}

void Stream::use()
{
	contents = fs::contents<vector<char>>(fileName);
}

std::string Stream::name()
{
	return fileName;
}

unique_ptr<src::Stream> stream(const string &fileName)
{
	return unique_ptr<src::Stream>{new Stream{string{fileName}}};
}

unique_ptr<src::Stream> stream(string &&fileName)
{
	return unique_ptr<src::Stream>{new Stream{move(fileName)}};
}

} // file

} // src