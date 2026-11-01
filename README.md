# errno-names

Part of [Kronuz C++ Libraries](https://github.com/Kronuz/cpp-libraries), a collection of small, reusable C++ libraries.

[![CI](https://github.com/Kronuz/errno-names/actions/workflows/ci.yml/badge.svg)](https://github.com/Kronuz/errno-names/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Header-only](https://img.shields.io/badge/header--only-yes-green.svg)

Header-only errno names for C++17: turn an errno value into its symbol, such as EINVAL, and the libc's description.
One list of `#ifdef`-guarded symbols, expanded as X-macros, builds a table of
every errno the target platform defines. Reach for it when a log line should
read `EINVAL (Invalid argument)` instead of `22`.

## ✨ Highlights

- **The name you grep for.** `error::name(EINVAL)` is `"EINVAL"`, the same in
  every locale.
- **The libc's description.** `error::description(e)` is the `strerror_r` text
  for `e`, captured when the table is built.
- **Built for the platform.** `errnos.h` lists 150 symbols from Linux and
  macOS; each is `#ifdef`-guarded, so the table holds exactly the ones the target
  defines.
- **Unknown values are not errors.** A number with no symbol reads `"UNKNOWN"`,
  and one outside the table describes itself as `"Unknown error"`.
- **O(1) lookups.** Both tables are built once, on first use; every call returns
  a reference to a stored `std::string`.

## 🚀 Quick start

```cpp
#include "error.hh"

#include <cerrno>
#include <cstdio>

int main() {
	int e = ENOENT;  // say, from a failed open()
	std::printf("open failed: %s (%s)\n", error::name(e).c_str(), error::description(e).c_str());
	// open failed: ENOENT (No such file or directory)
}
```

[`examples/demo.cc`](examples/demo.cc) prints a table of common errnos and the
whole compiled-in list.

## 📦 Installation

Requires a C++17 compiler and CMake 3.14 or newer; no dependencies. CI builds it
on Linux (x86_64, arm64), macOS (arm64, x86_64), and FreeBSD, plus ASan, UBSan,
and TSan builds.

```cmake
include(FetchContent)
FetchContent_Declare(errno_names
  GIT_REPOSITORY https://github.com/Kronuz/errno-names.git
  GIT_TAG        a5ab101151d32ab9502c2cdfd2b4022aefd970ae)  # or main for the tip
FetchContent_MakeAvailable(errno_names)

target_link_libraries(my_app PRIVATE errno_names::errno_names)
```

```cpp
#include "error.hh"
```

Copying `error.hh` and `errnos.h`, side by side, onto the include path works too.

## 🎯 When to use it

Use it in logs, error messages, and reports of failed system calls, where the
symbol is what a reader searches for.

Not for:

- **Error handling.** It maps integers to strings; `std::error_code` carries an
  error with its category.
- **Descriptions alone.** `std::system_category().message(e)` already returns
  them; the symbol is what this library adds.
- **Telling aliases apart.** When two symbols share a value
  (`EAGAIN == EWOULDBLOCK`), `name()` returns the one listed last in `errnos.h`.
- **Descriptions under glibc.** g++ and clang++ on Linux define `_GNU_SOURCE`,
  which selects glibc's `char*`-returning `strerror_r`; `error.hh` reads that
  result as a failure, so `description(ENOENT)` is `"Unknown error: 2"` there.
- **Locale switches.** Descriptions follow the libc locale at the first call and
  do not change afterwards.

## 📚 Documentation

- [docs/API.md](docs/API.md): `name`, `description`, and their fallbacks.
- [ARCHITECTURE.md](ARCHITECTURE.md): the X-macro list and how the tables are
  built.
- [AGENTS.md](AGENTS.md): repository map, adding an errno, and traps.
- [Catalog entry](https://github.com/Kronuz/cpp-libraries/blob/main/AGENTS.md#errno-names)
  in Kronuz C++ Libraries.

## 🛠️ Building and testing

```sh
cmake -B build
cmake --build build
ctest --test-dir build
./build/errno_names_demo
```

## 🤝 Contributing

Issues and pull requests are welcome. Read [AGENTS.md](AGENTS.md) and the
catalog's [rules of engagement](https://github.com/Kronuz/cpp-libraries/blob/main/AGENTS.md#rules-of-engagement)
first; CI must pass on every platform.

## 📄 License

MIT, Copyright (c) 2026 Germán Méndez Bravo (Kronuz) and Copyright (c) 2015-2020
Dubalu LLC. See [LICENSE](LICENSE). Extracted from
[Xapiand](https://github.com/Kronuz/Xapiand).
