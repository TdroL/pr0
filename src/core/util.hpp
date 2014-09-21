#ifndef UTIL_HPP
#define UTIL_HPP

namespace util {

#define UTIL_STRINGIFY_IMPL(x) #x
#define UTIL_STRINGIFY(x) UTIL_STRINGIFY_IMPL(x)

#define UTIL_CONCAT2_IMPL(one, two) one##two
#define UTIL_CONCAT2(one, two) UTIL_CONCAT2_IMPL(one, two)

#define UTIL_CONCAT3_IMPL(one, two, three) one##two##three
#define UTIL_CONCAT3(one, two, three) UTIL_CONCAT3_IMPL(one, two, three)

#define UTIL_CONCAT4_IMPL(one, two, three, four) one##two##three##four
#define UTIL_CONCAT4(one, two, three, four) UTIL_CONCAT4_IMPL(one, two, three, four)

#define UTIL_CONCAT5_IMPL(one, two, three, four, five) one##two##three##four##five
#define UTIL_CONCAT5(one, two, three, four, five) UTIL_CONCAT5_IMPL(one, two, three, four, five)

#define UTIL_CONCAT6_IMPL(one, two, three, four, five, six) one##two##three##four##five##six
#define UTIL_CONCAT6(one, two, three, four, five, six) UTIL_CONCAT6_IMPL(one, two, three, four, five, six)

#define UTIL_CONCAT7_IMPL(one, two, three, four, five, six, seven) one##two##three##four##five##six##seven
#define UTIL_CONCAT7(one, two, three, four, five, six, seven) UTIL_CONCAT7_IMPL(one, two, three, four, five, six, seven)

#if defined(DEBUG) || defined (_DEBUG)
	#define UTIL_DEBUG
#else
	#define UTIL_DEBUG if (false)
#endif

} // util

#endif