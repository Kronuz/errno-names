// Both strerror_r variants are read correctly, on any platform: the XSI one returns an int and fills the
// buffer; the GNU one returns the text, which need not be in the buffer.
// Build: c++ -std=c++17 -I.. test_strerror_variants.cc -o test_strerror_variants && ./test_strerror_variants
#undef NDEBUG  // the checks must run in every build type
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include "error.hh"

int main() {
	using error::detail::describe_with;

	// XSI: 0 means the buffer holds the text.
	auto xsi_ok = [](int, char* buf, std::size_t n) -> int {
		std::snprintf(buf, n, "xsi text");
		return 0;
	};
	assert(describe_with(xsi_ok, 7) == "xsi text");

	// XSI: a non-zero result (EINVAL, ERANGE) means the buffer is not trusted.
	auto xsi_fail = [](int, char* buf, std::size_t n) -> int {
		std::snprintf(buf, n, "untrusted");
		return 22;
	};
	assert(describe_with(xsi_fail, 7) == "Unknown error: 7");

	// GNU: the result points at static text and leaves the buffer alone.
	auto gnu_static = [](int, char*, std::size_t) -> char* {
		static char text[] = "gnu static text";
		return text;
	};
	assert(describe_with(gnu_static, 7) == "gnu static text");

	// GNU: the result points into the buffer.
	auto gnu_buffer = [](int, char* buf, std::size_t n) -> char* {
		std::snprintf(buf, n, "gnu buffer text");
		return buf;
	};
	assert(describe_with(gnu_buffer, 7) == "gnu buffer text");

	// GNU: a const char* result works as well, and a null one degrades.
	auto gnu_const = [](int, char*, std::size_t) -> const char* { return "gnu const text"; };
	assert(describe_with(gnu_const, 7) == "gnu const text");
	auto gnu_null = [](int, char*, std::size_t) -> const char* { return nullptr; };
	assert(describe_with(gnu_null, 7) == "Unknown error: 7");

	std::printf("errno-names strerror variants OK\n");
	return 0;
}
