#ifndef UTIL_HPP
#define UTIL_HPP

namespace util {

#define UTIL_CONCAT2_IMPL(one, two) one##two
#define UTIL_CONCAT2(one, two) UTIL_CONCAT2_IMPL(one, two)

#define UTIL_CONCAT3_IMPL(one, two, three) one##two##three
#define UTIL_CONCAT3(one, two, three) UTIL_CONCAT2_IMPL(one, two, three)

#if defined(DEBUG) || defined (_DEBUG)
	#define UTIL_DEBUG
#else
	#define UTIL_DEBUG if (false)
#endif

} // util

#endif