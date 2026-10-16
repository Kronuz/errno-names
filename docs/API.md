# errno-names API reference

Both functions live in namespace `error` and return a reference to a stable `std::string` (no allocation per call after the first build).

- `error::name(int errnum)` -> the symbolic name, e.g. `"EINVAL"`. Returns `"UNKNOWN"` if `errnum` is out of range or has no symbol in the table.
- `error::description(int errnum)` -> the human description, produced via `strerror_r` when the table is built. Returns `"Unknown error"` if `errnum` is out of range or has no entry.

Both tables are built once, lazily, on the first call into the library, stored as `std::array<std::string>`, and reused afterward, so lookups are O(1) array indexing.

## Notes & caveats

- On platforms where two errnos share the same integer value (for example `EAGAIN == EWOULDBLOCK` on Linux, or `ENOTSUP == EOPNOTSUPP` on some systems), `name()` returns whichever symbol is listed *last* in `errnos.h`, since the later assignment overwrites the earlier one when the table is populated.
- Only errnos that the platform actually defines are compiled in, because each entry in `errnos.h` is guarded by `#ifdef`. The table therefore reflects the build target, not a hardcoded universal list.
- The table is sized to at least 256 entries, growing if any defined errno value is larger.
- `description()` strings come from `strerror_r` and so follow the libc's locale at the time the table is first built.
