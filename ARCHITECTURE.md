# Architecture

errno-names is two files and one idea. The idea is to keep a single, canonical list of errno symbols and drive everything from it: the size of the lookup tables, the names table, and the descriptions table. The mechanism that makes one list serve all three purposes is an X-macro.

## The two files

- `errnos.h` is nothing but a list. Each line is a guarded invocation of a macro called `__ERRNO`:

  ```cpp
  #ifdef EAGAIN
      __ERRNO(EAGAIN)
  #endif
  ```

  The file never defines `__ERRNO`. It assumes whoever includes it has defined the macro first, and it leaves it undefined for the next includer. Every symbol is wrapped in an `#ifdef`, so a symbol that the platform does not define simply contributes nothing.

- `error.hh` is the logic. It includes `errnos.h` more than once, each time with a different definition of `__ERRNO`, so the same list of symbols gets compiled into different code each pass.

## The X-macro technique

An X-macro is a list of items expressed as repeated calls to a macro you have not defined yet. The list lives in one place; each *use* of the list redefines the macro to do something specific, includes the list, then undefines the macro. Because the list is textually re-expanded each time, one declaration of the data drives many derived constructs with no risk of them drifting out of sync.

`error.hh` uses the list three times.

First pass, size the tables. The macro turns each symbol into a comparison that tracks the largest errno value seen, evaluated in a `constexpr` lambda so the result is a compile-time constant:

```cpp
constexpr size_t num_errors = []{
    std::size_t max = 0;
    #define __ERRNO(name) if (name + 1 > max) max = name + 1;
    #include "errnos.h"
    #undef __ERRNO
    return max < 256 ? 256 : max;
}();
```

After this pass, `num_errors` is large enough to index by any defined errno value, with a floor of 256.

Second pass, fill the names table. Now `__ERRNO` maps each symbol to a string assignment, using the stringizing operator `#name` to turn the token `EAGAIN` into the literal `"EAGAIN"`, indexed by the symbol's integer value:

```cpp
std::array<std::string, num_errors> _names{};
_names[0] = "UNDEFINED";
#define __ERRNO(name) _names[name] = #name;
#include "errnos.h"
#undef __ERRNO
```

The same list, the same symbols, but this expansion writes `_names[EAGAIN] = "EAGAIN";` and so on for every symbol the platform defines.

The descriptions table is filled differently. Rather than a third X-macro pass, the code loops over every index from 0 to `num_errors` and calls `strerror_r(i, ...)`, storing the result (or a `"Unknown error: N"` fallback). The errno list is not needed here because `strerror_r` already maps integers to descriptions; the list's job was to recover the *names*, which the libc does not expose.

Both arrays are captured in a `static const auto` initialized by an immediately-invoked lambda, so the whole build happens exactly once, on the first call into `detail::errnos()`, and is reused thereafter.

## Why this is portable

Portability falls out of the `#ifdef` guards around every entry in `errnos.h`. On a Linux build, the Linux-only symbols (`EADV`, `EBADE`, `EL2HLT`, and so on) are defined by the system headers, so their lines compile in; the macOS-only `EQFULL` is not defined, so its line vanishes before the compiler ever sees it. On macOS the reverse happens. The list in `errnos.h` is the union of Linux, macOS, and BSD errnos, but each build compiles only the subset the platform actually has. There is no per-platform configuration and no separate tables to maintain.

## Lookups

`name()` and `description()` are bounds-checked array indexing into the prebuilt arrays. After the one-time build, every lookup is O(1): check range, read the slot, and if the slot is empty return the `"UNKNOWN"` / `"Unknown error"` sentinel. The functions return a reference to a string with static storage duration, so repeated calls do not allocate.

## Design decisions

- One list, derived tables. Keeping `errnos.h` as a pure macro list means adding or removing a symbol is a one-line change that automatically flows into sizing and the names table. There is no parallel list to keep in step.
- Lazy, build-once. The tables cost a bit of memory and a one-time `strerror_r` sweep, paid on first use. For a logging helper, that is the right trade: callers do not pay setup cost unless they actually use it, and they pay it only once.
- Return by reference to a stable string. Avoids per-call allocation, at the cost of the strings living for the program's lifetime.

## Limitations

- Aliased errnos. When two symbols share a value (`EAGAIN == EWOULDBLOCK` on Linux), the second pass writes both into the same slot, so the *last* one listed in `errnos.h` wins for `name()`. The order in the file therefore decides which symbol is reported for an aliased value.
- Names only, not descriptions, come from the list. The description side leans entirely on `strerror_r`, so its strings and locale behavior are the libc's, not this library's.
- Slot 0 is special-cased to `"UNDEFINED"` because no real errno is 0, and the names array is otherwise default-initialized to empty strings that signal "no symbol here."
