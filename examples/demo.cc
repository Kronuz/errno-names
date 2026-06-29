// A runnable tour of errno-names.
//
// Build (when this repo is the top-level project):
//   cmake -B build && cmake --build build && ./build/errno_names_demo
//
// The one idea worth taking away: the C library gives you strerror (the human
// description) but never the symbolic *name* of an errno constant. errno-names
// fills that gap: error::name(EINVAL) is "EINVAL", the token a developer greps
// for, stable across locales, alongside error::description(EINVAL), which is the
// libc's localized "Invalid argument". This demo looks up a handful of errnos
// both ways, sweeps the whole compiled-in table (only the errnos this platform
// actually defines), shows the graceful fallback for an unknown value, and the
// log line shape the library exists to produce.
#include <cerrno>
#include <cstdio>

#include "error.hh"  // error::name(int), error::description(int)

static void rule(const char* title) {
	std::printf("\n\033[1m── %s ──\033[0m\n", title);
}

int main() {
	std::puts("errno-names demo");

	// --- 1. value -> name: the symbol the C library will not give you ---------
	rule("error::name(): an errno integer to its symbolic name");
	// strerror would hand you the description for these; name() hands you the
	// constant's spelling instead. EAGAIN/ENOENT/EINVAL/... are macros that
	// expand to this platform's integer values.
	const int sample[] = { EAGAIN, ENOENT, EINVAL, EACCES, EPERM, ENOMEM, EEXIST, EBADF };
	for (int e : sample) {
		std::printf("  %-3d -> %-8s  (%s)\n",
			e, error::name(e).c_str(), error::description(e).c_str());
	}

	// --- 2. why the name, not just the description ----------------------------
	rule("name vs. description: one is stable, one is localized");
	// The description comes from strerror_r and follows the libc locale; the name
	// is the constant's spelling and is what shows up in man pages and code.
	std::printf("  EINVAL  name=\"%s\"   description=\"%s\"\n",
		error::name(EINVAL).c_str(), error::description(EINVAL).c_str());
	std::printf("  ENOENT  name=\"%s\"   description=\"%s\"\n",
		error::name(ENOENT).c_str(), error::description(ENOENT).c_str());

	// --- 3. sweep the table: every errno this build actually compiled in ------
	rule("iterate: each defined errno on this platform (value -> name)");
	// errnos.h is the union of Linux/macOS/BSD symbols, each #ifdef-guarded, so a
	// build compiles only the subset the platform defines. Sweeping the integer
	// range and printing the slots with a real name (not the "UNKNOWN" sentinel)
	// walks exactly that subset, in value order.
	int defined = 0;
	for (int e = 1; e < 256; ++e) {
		const std::string& n = error::name(e);
		if (n != "UNKNOWN") {
			std::printf("  %3d -> %-16s %s\n", e, n.c_str(), error::description(e).c_str());
			++defined;
		}
	}
	std::printf("  (%d errno symbols compiled in for this platform)\n", defined);

	// --- 4. the graceful fallback ---------------------------------------------
	rule("unknown errno: both lookups degrade instead of throwing");
	// Out of range or no symbol in the table: name() returns "UNKNOWN",
	// description() returns "Unknown error". Neither throws.
	std::printf("  error::name(-12345)      -> \"%s\"\n", error::name(-12345).c_str());
	std::printf("  error::description(99999) -> \"%s\"\n", error::description(99999).c_str());

	// --- 5. the log line this library exists to produce -----------------------
	rule("the payoff: a readable syscall-failure log line");
	// The shape a caller actually writes: capture errno after a failing call,
	// then print symbol + description together instead of a bare integer.
	errno = 0;
	FILE* f = std::fopen("/no/such/path/at/all", "r");  // fails, sets errno
	if (!f) {
		int e = errno;
		std::printf("  fopen failed: %s (%s)  [errno %d]\n",
			error::name(e).c_str(), error::description(e).c_str(), e);
	}

	std::puts("\ndone.");
	return 0;
}
