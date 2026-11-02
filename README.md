# 🧚 printf-fairy

A tester for 42's **ft_printf** (subject v12.1). About 22,000 formats compared byte for byte with the real `printf`, return values included, plus every check the subject asks for, in about 1.5 seconds.

Part of a trio with [libft-fairy](https://github.com/sys-harmony/libft-fairy) and [gnl-fairy](https://github.com/sys-harmony/gnl-fairy): same display, same verdicts, same engine.

## 🚀 Usage

Clone it inside your ft_printf repository, then run it:

```bash
git clone https://github.com/sys-harmony/printf-fairy.git
./printf-fairy/run.sh          # failing tests only
./printf-fairy/run.sh -v       # every test
```

The exit status is `0` when everything passed (warnings included), `1` otherwise, so it fits in a script or a CI.

## 📊 Output

```
╔════════════════════════════════════════════╗
║              PRINTF-FAIRY 🧚               ║
╚════════════════════════════════════════════╝

🔖 Version ............................. Bonus
📂 Files ................................ Done
📖 README ............................... Done
📝 Norm ................................. Done
📋 Prototype ............................ Done
🔧 Makefile ............................. Done
📦 Build ................................ Done
🔍 External functions ................... Done
🔨 Test build ........................... Done
📦 Bonus build .......................... Done
🔍 Bonus external functions ............. Done
🔨 Bonus test build ..................... Done
🧪 Tests .............................. Failed

── ft_printf conversions ─────────────────────
[KO] %c, every byte from 0 to 255
     ↳ format   "[%c]"
       expected "[\x00]"  ret 3
       got      "[]"  ret 2
[KO] %p: NULL, top bit set, stack, heap
     ↳ format   "[%p]"
       expected "[0xffffffffffffffff]"  ret 20
       got      "[0x-1]"  ret 6
🧪 Bonus tests ......................... Done
🧹 Cleanup .............................. Done

╔════════════════════════════════════════════╗
║            OH NO... YOU FAILED!            ║
╚════════════════════════════════════════════╝
   ✘ Tests · 2/49 tests failed · 1.37s
```

Only what did not pass is listed, each time with its reason: the format, the bytes expected and the bytes written (non-printable ones as `\xHH`, long outputs cut around the first difference), a crash signal, a timeout, or the Valgrind report.

## ⚖️ Failures and warnings

A **failure** is a bug or a clear subject violation. A **warning** is something debatable: worth a look, maybe a question from your evaluator, never a failure. A working ft_printf passes.

Warnings, for ft_printf:

- a combination the C standard leaves undefined (flag `0` on `%s`, `+` on `%u`, a precision on `%p`, `%5%`...) that differs from glibc
- a return value other than `-1` when stdout is closed (write errors are not in the subject)
- a README that does not look like the subject's description
- project files that git does not track, hence not graded
- `norminette` or `valgrind` missing: that check did not run

## 🧪 What is checked

| Step | What |
|---|---|
| 🔖 Version | Bonus when the Makefile has a `bonus` rule |
| 📂 Files | a Makefile and `ft_printf.h`; project files not committed (warning) |
| 📖 README | there (failure), with the italic first line, *Description*, *Instructions*, *Resources*, how AI was used and the algorithm (warnings) |
| 📝 Norm | `norminette` on every graded `.c` and `.h`, `libft/` included |
| 📋 Prototype | the compiler checks `ft_printf.h` against `int ft_printf(const char *, ...)` |
| 🔧 Makefile | `$(NAME)`, `all`, `clean`, `fclean`, `re`; `cc -Wall -Wextra -Werror` on every compilation `make -n` shows, sub-makes included; `ar`, no `libtool`; no wildcard |
| 📦 Build | a build from scratch, then `make` again must not relink; same for `make bonus` |
| 🔍 External functions | in `libftprintf.a` itself: `malloc`, `free`, `write` only. `memset`, `memcpy` and `__stack_chk_fail` the compiler emits on its own are allowed, unless written in the sources |
| 🧪 Tests | below, against the mandatory build, then everything again against the bonus build |

**Mandatory conversions:** `%c` for every byte from 0 to 255; `%s` with empty, whitespace, high bytes, a format inside, `NULL`, 3000 characters; `%p` with `NULL`, the top bit set, stack and heap addresses; `%d %i %u %x %X` swept over 0, ±1, powers of 10 and 16, `INT_MIN`, `UINT_MAX`, negative arguments; `%%`. Then formats: empty, text only, 5000 characters, every conversion at once, 20 in a row.

**Robustness:** each `malloc` failing in turn (no crash allowed), stdout closed (warning).

**Bonus:** one test per classic pitfall (`0` ignored with `-` or a precision, `#` with 0, `+` beats space, `.0` on 0, `%10s` on `NULL`...), the undefined combinations (warnings), then **exhaustive sweeps**: every combination of 15 flag sets × 5 widths × 7 precisions × values, for each conversion. `defined()` sorts them: what the standard defines must match glibc, the rest only warns.

**Leaks:** Valgrind on short, 8 KB, `NULL` and many-argument prints, huge widths and precisions; any leak, invalid access or uninitialised value fails.

## 🔍 How it works

- `CHECK(fmt, ...)` runs `ft_printf`, then `printf`, with the same arguments, captures what each writes to stdout through a pipe and compares the raw bytes, so a `'\0'` cannot end the comparison early, and the return values.
- Every test runs in its own process with a timeout: a crash or an infinite loop fails that test only, and the remaining timeouts shrink to 1 second.
- The mandatory tests run in the background while the bonus builds; norminette starts at once. Every core of the machine is used.
- `-Wl,--wrap=malloc` routes the library's `malloc` through the tester, which can make the n-th one fail.

## 📋 Requirements

Linux with `bash` 4.3+, `cc`, `make`, `nm`, `valgrind`, `norminette`, `perl` and coreutils. The reference is glibc's `printf` (`(nil)` for a `NULL` `%p`), as on 42's machines.

## 📁 Structure

```
printf-fairy/
├── run.sh               # the steps; its "fairy core" is shared by the trio
├── utils.c              # capture, comparison, malloc wrapper; the C fairy core
├── printf_fairy.h       # CHECK, CHECK_UB and SWEEP
├── basic_tests.c        # mandatory conversions
├── basic_tests_bonus.c  # flags, undefined combinations, sweeps
├── leak_tests.c         # under Valgrind
└── leak_tests_bonus.c   # under Valgrind, bonus paths
```

Adding a test: write a function of `CHECK()` calls and add it to its group's table.

## 💖 Credits

Created with ✨ by **sys-harmony**. Issues and new edge cases are welcome.
