# errno-names

Map an `errno` value to its symbolic name (`"EAGAIN"`, `"ENOENT"`, ...) and to its human description, across Linux, macOS, and BSD.

## What it is

The C standard library gives you `strerror` / `strerror_r`, which turn an `errno` value into a human description like `"Invalid argument"`. What it does not give you is the symbolic *name* of the constant, `"EINVAL"`. errno-names fills that gap: a header-only C++17 library that maps an `errno` integer to both its symbolic name and its description. It builds a compile-time list of every errno the platform actually defines, so a log line can read `EINVAL (Invalid argument)` instead of a bare number.

## When to use it / when not

Use it when you are logging or reporting system-call failures and you want the symbolic name in the output, not just the description or the raw integer. The name is what a developer greps for, what shows up in man pages, and what is stable across locales, so `EINVAL` is more useful in a log than `"Invalid argument"` (which is localized) or `22` (which is platform-dependent).

Skip it if a bare `strerror` description is enough for your audience, or if you are not in a position to add a header to the build. It is a small convenience, not infrastructure. It is also not a general error-handling framework: it maps `errno` integers to strings and nothing more.

## Install

Header-only. Two files, `error.hh` and `errnos.h`, must sit next to each other (`error.hh` includes `errnos.h` with a relative path). Drop them into your include path and include the one header:

```cpp
#include "error.hh"
```

Requires C++17.

### CMake (FetchContent)

The repo ships an `INTERFACE` target named `errno_names`:

```cmake
include(FetchContent)
FetchContent_Declare(
  errno_names
  GIT_REPOSITORY https://github.com/Kronuz/errno-names.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(errno_names)

target_link_libraries(your_target PRIVATE errno_names)
```

The target sets `cxx_std_17` and adds the repo root to your include path, so `#include "error.hh"` resolves.

## Usage

```cpp
#include <cerrno>
#include <cstdio>
#include "error.hh"

int main() {
    // Symbolic name lookup.
    std::printf("%s\n", error::name(EINVAL).c_str());         // "EINVAL"
    std::printf("%s\n", error::name(ENOENT).c_str());         // "ENOENT"

    // Description (via strerror_r, so it matches what the libc reports).
    std::printf("%s\n", error::description(EINVAL).c_str());  // "Invalid argument"

    // Unknown errno: both lookups degrade gracefully instead of throwing.
    std::printf("%s\n", error::name(-12345).c_str());         // "UNKNOWN"
    std::printf("%s\n", error::description(99999).c_str());   // "Unknown error"

    // Typical logging shape.
    int e = errno;  // captured after some failing syscall
    std::printf("syscall failed: %s (%s)\n",
                error::name(e).c_str(), error::description(e).c_str());
    return 0;
}
```

## API reference

Both functions live in namespace `error` and return a reference to a stable `std::string` (no allocation per call after the first build).

- `error::name(int errnum)` -> the symbolic name, e.g. `"EINVAL"`. Returns `"UNKNOWN"` if `errnum` is out of range or has no symbol in the table. (`error.hh:70`)
- `error::description(int errnum)` -> the human description, produced via `strerror_r` when the table is built. Returns `"Unknown error"` if `errnum` is out of range or has no entry. (`error.hh:85`)

Both tables are built once, lazily, on the first call into the library, stored as `std::array<std::string>`, and reused afterward, so lookups are O(1) array indexing (`error.hh:37`).

## Build & test

Header-only, so there is nothing to build for use. To run the smoke test:

```sh
c++ -std=c++17 -I. test/test.cc -o test/test && ./test/test
# or with CMake:
cmake -B build && cmake --build build && ctest --test-dir build
```

The test (`test/test.cc`) checks that known errnos resolve to their names, that descriptions are non-empty, and that an out-of-range errno falls back to `"UNKNOWN"`.

## Notes & caveats

- On platforms where two errnos share the same integer value (for example `EAGAIN == EWOULDBLOCK` on Linux, or `ENOTSUP == EOPNOTSUPP` on some systems), `name()` returns whichever symbol is listed *last* in `errnos.h`, since the later assignment overwrites the earlier one when the table is populated.
- Only errnos that the platform actually defines are compiled in, because each entry in `errnos.h` is guarded by `#ifdef`. The table therefore reflects the build target, not a hardcoded universal list.
- The table is sized to at least 256 entries, growing if any defined errno value is larger (`error.hh:39`).
- `description()` strings come from `strerror_r` and so follow the libc's locale at the time the table is first built.

## Provenance

Extracted from [Xapiand](https://github.com/Kronuz/Xapiand), where it makes I/O error logs readable by printing the errno symbol alongside the description.

## License

MIT. Copyright (c) 2015-2019 Dubalu LLC. See [LICENSE](LICENSE).
