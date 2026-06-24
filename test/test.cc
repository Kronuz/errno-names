// Smoke test for the standalone errno-names library.
// Build: c++ -std=c++17 -I.. test.cc -o test && ./test
#include <cassert>
#include <cerrno>
#include <cstdio>
#include "error.hh"

int main() {
	// Symbolic name lookup (use errnos with unique values to avoid alias ambiguity).
	assert(error::name(EINVAL) == "EINVAL");
	assert(error::name(ENOENT) == "ENOENT");

	// Descriptions are non-empty and unknown errnos degrade gracefully.
	assert(!error::description(EINVAL).empty());
	assert(error::name(-12345) == "UNKNOWN");

	std::printf("errno-names OK: EINVAL = %s (%s)\n",
	            error::name(EINVAL).c_str(), error::description(EINVAL).c_str());
	return 0;
}
