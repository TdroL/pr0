#ifndef NGN_FS_HPP
#define NGN_FS_HPP

#include <string>
#include <vector>
#include <fstream>

namespace ngn
{

namespace fs
{

extern std::vector<std::string> searchDirectories;

template<class T>
T contents(const std::string &fileName, bool throws = true);
std::string contents(const std::string &fileName, bool throws = true);
std::string find(const std::string &fileName, bool throws = true);
std::string ext(const std::string &fileName);
size_t size(const std::string &fileName);
size_t size(std::ifstream &in);

} // fs

} // ngn

#endif