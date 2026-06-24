# errno-names

Map an `errno` value to its **symbolic name** (`"EAGAIN"`, `"ENOENT"`, …) and to
its human description, across Linux/macOS/BSD. The standard library gives you
`strerror` (the description) but not the *name* — this fills that gap with a
compile-time table of every errno the platform defines.

Extracted from [Xapiand](https://github.com/Kronuz/Xapiand), where it makes I/O
error logs readable.

## Usage

```cpp
#include "error.hh"

error::name(EINVAL);          // "EINVAL"
error::description(EINVAL);    // "Invalid argument"  (via strerror_r)
error::name(-12345);          // "UNKNOWN"  (graceful fallback)
```

Both tables are built lazily on first use (`std::array<std::string>`), so lookups
are O(1).

## Build & test

Header-only (`error.hh` + `errnos.h`). To run the smoke test:

```sh
c++ -std=c++17 -I. test/test.cc -o test/test && ./test/test
# or: cmake -B build && cmake --build build && ctest --test-dir build
```

Requires C++17.

## Notes

- On platforms where two errnos share a value (e.g. `EAGAIN` == `EWOULDBLOCK` on
  Linux), `name()` returns whichever symbol is listed last in `errnos.h`.

## License

MIT — see [LICENSE](LICENSE).
