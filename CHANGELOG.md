# Changelog

Notable changes to `errno-names`, newest first. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); the library has no releases, so
entries are dated.

Upgrading a pin from before 2026-10-30: [the consumer upgrade note](https://github.com/Kronuz/cpp-libraries/blob/main/docs/UPGRADING-2026-10.md#errno-names) lists what changes for callers, how to find affected inputs, the commits not to pin, and the speedup.

## 2026-11-01

### Fixed

- `error::description()` returned `"Unknown error: N"` for every errno on Linux. g++ and
  clang++ on glibc define `_GNU_SOURCE`, which selects the GNU `strerror_r`: it returns the
  text as a `char*`, which `error.hh` compared with 0 as if it were the XSI status. The
  table is now filled through two overloads on the result type, so the XSI `int` and the GNU
  `char*` forms are both read; a null or non-zero result still gives `"Unknown error: N"`.
  `description(ENOENT)` is `"No such file or directory"` on both platforms.

### Tests

- `errno_names_descriptions` compares `description(e)` with `std::strerror(e)` for every slot
  of the table (0 to 255) and checks four well-known texts. It fails on glibc with the old
  header (`errno 0 (UNDEFINED): description "Unknown error: 0", libc "Success"`) and passes on
  macOS with either.
- `errno_names_strerror_variants` feeds `detail::describe_with` stand-ins for both
  `strerror_r` forms (status code, static text, text in the buffer, null), so each reading is
  covered on every platform.
