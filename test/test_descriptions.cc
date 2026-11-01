// Descriptions equal the C library's text for every errno slot, whichever strerror_r the platform selects.
// The oracle is std::strerror, which is independent of the strerror_r variant.
// Build: c++ -std=c++17 -I.. test_descriptions.cc -o test_descriptions && ./test_descriptions
#undef NDEBUG  // the checks must run in every build type
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include "error.hh"

int main() {
	// Every slot of the table, named or not, reads as the libc reads it.
	int checked = 0;
	for (int e = 0; e < 256; ++e) {
		const std::string expected = std::strerror(e);
		const std::string& got = error::description(e);
		if (got != expected) {
			std::printf("errno %d (%s): description \"%s\", libc \"%s\"\n", e, error::name(e).c_str(), got.c_str(), expected.c_str());
			return 1;
		}
		++checked;
	}

	// Well-known texts, in the C locale of both glibc and the BSD libc.
	assert(error::description(EINVAL) == "Invalid argument");
	assert(error::description(ENOENT) == "No such file or directory");
	assert(error::description(EACCES) == "Permission denied");
	assert(error::description(EPIPE) == "Broken pipe");

	// A named errno never degrades to the fallback text.
	assert(error::description(ENOENT).find("Unknown error") == std::string::npos);

	// Out of range keeps the sentinel.
	assert(error::description(-1) == "Unknown error");
	assert(error::description(1 << 20) == "Unknown error");

	std::printf("errno-names descriptions OK: %d slots match the libc\n", checked);
	return 0;
}
