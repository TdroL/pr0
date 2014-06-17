#include "fs.hpp"
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace ngn
{

namespace fs
{

using namespace std;

vector<string> searchDirectories{
	"",
	"assets/",
	"assets/fonts/",
	"assets/meshes/",
	"assets/shaders/",
	"assets/textures/",
};

// http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
template<>
string contents(const string &fileName, bool throws)
{
	ifstream in(find(fileName), ios::in | ios::binary);

	if (in && in.good())
	{
		string contents{};
		size_t fileSize = size(in) / sizeof(char);
		contents.reserve(fileSize);
		contents[fileSize - 1] = '\0';

		contents.assign(istreambuf_iterator<char>(in), istreambuf_iterator<char>());

		return contents;
	}

	if (throws)
	{
		throw string{"ngn::fs::contents(\"" + fileName + "\") - failed to read the file"};
	}

	return string{};
}

template<>
vector<char> contents(const string &fileName, bool throws)
{
	ifstream in(find(fileName), ios::in | ios::binary);

	if (in && in.good())
	{
		vector<char> contents{};
		size_t fileSize = size(in) / sizeof(char);
		contents.reserve(fileSize);
		contents[fileSize - 1] = '\0';

		contents.assign(istreambuf_iterator<char>(in), istreambuf_iterator<char>());

		return contents;
	}

	if (throws)
	{
		throw string{"ngn::fs::contents(\"" + fileName + "\") - failed to read the file"};
	}

	return vector<char>{};
}

std::string contents(const std::string &fileName, bool throws)
{
	return contents<string>(fileName, throws);
}

string find(const string &fileName, bool throws)
{
	for (const auto &directory : searchDirectories)
	{
		string filePath = directory + fileName;
		ifstream in(filePath);

		if (in.is_open())
		{
			return filePath;
		}
	}

	if (throws)
	{
		throw string{"ngn::fs::find(\"" + fileName + "\") - could not find the file"};
	}

	return "";
}

string ext(const string &fileName)
{
	size_t pos = fileName.find_last_of('.');

	if (pos != string::npos && (pos + 1) < fileName.size())
	{
		return fileName.substr(pos + 1);
	}

	return "";
}

size_t size(const string &fileName)
{
	ifstream in(find(fileName), ios::binary);

	return size(in);
}

size_t size(ifstream &in)
{
	if (in && in.good())
	{
		in.seekg(0, ios::end);
		size_t length = in.tellg();
		in.seekg(0, ios::beg);

		return length;
	}

	return -1;
}

} // fs

} // ngn