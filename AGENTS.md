# AGENTS.md

Guidance for agents (and humans) working in this repo. It is small: two header files, a test, and a CMake shim. Read `ARCHITECTURE.md` for the X-macro design before changing either header.

## Repo map

- `error.hh` — the library. Namespace `error`, header-only. Builds the name and description tables lazily and exposes `error::name(int)` and `error::description(int)`.
- `errnos.h` — the canonical errno list. Pure X-macro list of `#ifdef Exxx __ERRNO(Exxx) #endif` lines. Defines no logic; included multiple times by `error.hh` with different `__ERRNO` definitions.
- `test/test.cc` — smoke test. `test/test_descriptions.cc` — every description equals `std::strerror`'s. `test/test_strerror_variants.cc` — both `strerror_r` forms, with stand-ins.
- `CMakeLists.txt` — declares the `errno_names` INTERFACE target and the `errno_names_test` test target.
- `README.md` — what it is, install, quick start; `docs/API.md` — the API reference and caveats; `ARCHITECTURE.md` — the design.

## Verification and benchmarks

The shared method is the catalog's
[PERFORMANCE.md](https://github.com/Kronuz/cpp-libraries/blob/main/docs/PERFORMANCE.md). From a
fresh clone:

```sh
cmake -B build && cmake --build build && ctest --test-dir build --output-on-failure
```

- **Tests.** `errno_names` (`test/test.cc`) is the smoke test; a passing run prints
  `errno-names OK: EINVAL = EINVAL (Invalid argument)`. Its checks are `assert`s, so leave `NDEBUG`
  off (no build type, or Debug); the other two `#undef NDEBUG`. `errno_names_descriptions` is the
  oracle test: `description(e)` equals libc's `std::strerror(e)` for every slot, 0 to 255, plus
  four known texts. `errno_names_strerror_variants` feeds `detail::describe_with` stand-ins for the
  XSI `int` and the GNU `char*` forms of `strerror_r` (status, static text, text in the buffer,
  null), so both readings run on every platform.
- **Revert check.** The glibc fix (`0ec0801`, CHANGELOG 2026-11-01) shows on Linux: with the old
  header, `errno_names_descriptions` fails at slot 0 (`Unknown error: 0` against `Success`). The
  stand-in test needs the new header, so build the oracle test alone:

  ```sh
  git show 0ec0801^:error.hh > error.hh
  cmake --build build --target errno_names_descriptions && ./build/errno_names_descriptions; git checkout error.hh
  ```

- **CI.** `.github/workflows/ci.yml` calls the catalog's shared
  [`cmake.yml`](https://github.com/Kronuz/cpp-libraries/blob/main/.github/workflows/cmake.yml):
  Linux x86_64 (GCC, Clang) and arm64 (GCC) on glibc, macOS arm64 and x86_64, and FreeBSD, at
  `-O2 -g` with asserts live, plus ASan+UBSan and TSan jobs over the same tests. The ASan+UBSan
  job, on Linux:

  ```sh
  CC=clang CXX=clang++ cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug \
    "-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS" \
    -DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined
  cmake --build build-asan && ASAN_OPTIONS=detect_leaks=1 ctest --test-dir build-asan --output-on-failure
  ```

- **Not here.** No differential test, mutation script, fuzz target or benchmark; the README's
  "O(1) lookups" is an index into a table built on first use.

## Conventions

- C++17, header-only. No `.cpp` files in the library, no link-time dependencies beyond the standard library.
- `error.hh` includes `errnos.h` by relative path, so the two files must ship together in the same directory.
- Indentation in the existing files is tabs; match it.
- Code blocks and string literals use double quotes.

## Invariants

- The `__ERRNO` contract. `errnos.h` calls a macro named `__ERRNO` that it never defines. Every includer must `#define __ERRNO(...)` before `#include "errnos.h"` and `#undef __ERRNO` after. Do not define `__ERRNO` inside `errnos.h`, and do not leave it defined after a pass.
- `errnos.h` stays a pure list. No logic, no includes, no state. Only guarded `__ERRNO(...)` lines (plus comments). Anything that needs to vary per use case belongs in `error.hh`, expressed as a different `__ERRNO` definition.
- Every symbol is guarded by `#ifdef`. This is what makes the list portable: a symbol the platform does not define must compile to nothing.
- Table sizing happens in the first X-macro pass and must stay a `constexpr` computation, with the 256-entry floor preserved.

## How to extend (adding a platform errno)

Add one guarded line to `errnos.h`, in the section that matches the platform (Linux-only, macOS-only, or both):

```cpp
#ifdef ESOMETHINGNEW
    __ERRNO(ESOMETHINGNEW)
#endif
```

That is the whole change. The first X-macro pass picks it up for sizing, the second for the names table; the descriptions table already covers all indices via `strerror_r`. Do not touch `error.hh` to add a symbol. After editing, run the test.

## Traps

- Aliased errnos. If two symbols share a value (e.g. `EAGAIN == EWOULDBLOCK`), the *last* one listed in `errnos.h` wins for `name()`, because the names pass overwrites the shared slot. If you need a specific symbol to win for an aliased value, order matters: list the one you want reported last.
- Forgetting the `#undef`. Leaving `__ERRNO` defined after an `#include "errnos.h"` will break the next pass or any later include. Each pass is `#define` ... `#include` ... `#undef`.
- `errno` value 0. Slot 0 is set to `"UNDEFINED"` by hand because no real errno is 0; do not assume it comes from the list.
- Don't add includes to `errnos.h`. It is included repeatedly and mid-expression; it must remain a bare list.

## Provenance

This library was extracted from [Xapiand](https://github.com/Kronuz/Xapiand). Keep it self-contained: changes should not reintroduce a dependency on Xapiand or any larger framework. License is MIT, Copyright (c) 2015-2019 Dubalu LLC.
