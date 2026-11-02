# 🧚 printf-fairy

A comprehensive tester for 42's **ft_printf** project, featuring memory leak detection, edge case testing, and rigorous validation against the original `printf`.

## ✨ Features

- ✅ Complete tests for all conversions (mandatory + bonus)
- 🔬 Byte-exact comparison with the real `printf` (output **and** return value)
- 🩺 Every failure shows the format, the expected output and what you printed
- ⏱️ Crashes and infinite loops are reported per test instead of killing the run
- 🧹 Clean execution (automatic cleanup of temporary files)
- 🔍 Automatic verification with **norminette**
- 📋 Prototype validation in project headers
- ⚙️ Makefile rules verification (NAME, all, clean, fclean, re, bonus, relink)
- 🔧 Compilation flags check (-Wall -Wextra -Werror)
- 🔎 External function usage verification
- 💧 Memory leak detection with **Valgrind**
- 🚨 Edge case testing (NULL, INT_MIN, UINT_MAX, null bytes, huge widths, malloc failure)
- 📊 Clear and colorful output
- 🔇 Optional verbose mode for detailed results

## 📋 Prerequisites

Before using printf-fairy, make sure you have installed:

- `cc` (with `--wrap` linker support, i.e. GNU ld)
- `make`
- `valgrind`
- `norminette` (the norm step is skipped if it is missing)
- `bash`
- `nm` and `timeout` (usually pre-installed)

## 🚀 Installation

1. Clone the repository inside your ft_printf directory:

```bash
git clone https://github.com/sys-harmony/printf-fairy.git
```

2. Navigate to the tester folder:

```bash
cd printf-fairy
```

3. Make the script executable:

```bash
chmod +x run.sh
```

## 🎯 Usage

### Run all tests

```bash
./run.sh
```

### Verbose mode (displays all tests, even passing ones)

```bash
./run.sh --verbose
# or
./run.sh -v
```

## 📊 Output

```
╔════════════════════════════════════╗
║           PRINTF-FAIRY 🧚          ║
╚════════════════════════════════════╝

📝 Checking norm...             Done
🔖 Checking version...         Bonus
📋 Checking prototype...        Done
⚙️  Checking Makefile...        Done
📦 Building ft_printf...        Done
🔍 Checking externals...        Done
🔨 Building tests...            Done
🧪 Running tests...           Failed

========================================
ft_printf %c
========================================
[KO]  Test null byte
      format:   "[%c]"
      expected: "[\x00]" (ret 3)
      got:      "[" (ret 3)

========================================
ft_printf %p
========================================
[KO]  Test extremes (-1, LONG_MIN, LONG_MAX)
      format:   "[%p]"
      expected: "[0xffffffffffffffff]" (ret 20)
      got:      "[0x-1]" (ret 6)
...

╔════════════════════════════════════╗
║        OH NO... YOU FAILED!        ║
╚════════════════════════════════════╝
```

By default only failed tests are listed, each with the format string, the expected output and yours (non-printable bytes shown as `\xHH`; long outputs are cut around the first difference). A crash shows the signal (`crashed: Segmentation fault`), an infinite loop shows `timeout after 5s`. Valgrind logs are displayed when a leak test fails. The script exits with status 1 when anything failed, so it can be used in scripts.

## 🧪 Tests Covered

### Pre-compilation Checks

- **Norminette**: Validates all .c and .h files (skipped with a warning if `norminette` is not installed)
- **Prototype**: Verifies `int ft_printf(const char *, ...)` in any project header (`*.h`), even split over several lines
- **Makefile**: Checks for required rules, proper compilation flags, wildcards in source lists, and that neither `make` nor `make bonus` relinks when run twice
- **External functions**: Reads `libftprintf.a` itself (so objects built in `obj/` or `libft/` are checked too) and ensures only allowed symbols are left undefined: `malloc`, `free`, `write`, plus `memset`/`memcpy` (emitted by clang for struct copies) and `__stack_chk_fail` (gcc's stack protector); `va_*` are builtins and invisible to `nm`

### Mandatory Conversions

#### %c (character)
- Basic ASCII
- Null byte (`'\0'`), alone and between other characters
- Low and high ASCII range

#### %s (string)
- Basic, empty, NULL
- Long strings

#### %p (pointer)
- NULL, small addresses, large addresses, stack addresses
- Extremes with the top bit set (`-1`, `LONG_MIN`, `LONG_MAX`), which catch pointers printed as signed numbers

#### %d / %i (signed decimal)
- Zero, positive, negative
- `INT_MAX`, `INT_MIN`

#### %u (unsigned decimal)
- Zero, positive, `UINT_MAX`
- Negative arguments (`-1`, `INT_MIN`), which must be read as unsigned

#### %x / %X (hexadecimal)
- Zero, basic values, `UINT_MAX`, negative arguments
- Mixed lower/upper case

#### %% (percent sign)
- Basic, with text, consecutive

#### Mix
- Multiple conversions in one call
- All types in one format

#### Robustness
- Text on a closed stdout: `ft_printf` must return `-1`, like an unbuffered `printf`. Empty output is not tested: glibc's `printf("")` returns `0` there, since nothing is written
- Malloc failure: the 1st, then 2nd, ... `malloc` returns `NULL` (through `-Wl,--wrap=malloc`); `ft_printf` may give up but must not crash. Implementations that never call `malloc` pass as is

Every test also checks the return value against `printf`.

### Bonus Flags

#### Flag `-` (left-justify)
- With every conversion type (`%c`, `%s`, `%d`, `%x`)

#### Flag `0` (zero-pad)
- Basic, with negative numbers, with `%x`
- Correctly ignored when `-` or precision are present

#### Flag `#` (alternate form)
- Basic `0x`/`0X` prefix
- Correctly omitted when value is zero
- With width and zero-padding
- With precision

#### Flag ` ` (space)
- With positive, negative, zero
- Correctly overridden by `+`

#### Flag `+` (force sign)
- With positive, negative, zero
- With zero-padding

#### Width
- Basic, too small (ignored), width 0
- With strings
- Huge width padding (1000 characters)

#### Precision
- Basic for `%d`
- Precision 0 with n=0 (empty output)
- String truncation
- Precision longer than string
- Width + precision combinations
- Huge precision (`.40`)

#### Combinations
- All flags + width + precision
- `-` + width + precision with `%s`
- Repeated flags
- Mixed format strings
- Negative values as unsigned with width, precision and `#`

#### Misc flags
These follow glibc where the C standard leaves the behaviour undefined:
- `%p` with width (`%20p`, `%-20p`) and zero-padding (`%010p`)
- Flag `0` ignored for `%s`
- `%s` NULL with width

#### Exhaustive Cartesian Sweeps
- Every flag × width × precision × value combination for `%d %i %u %x %X %c %s %p` (about 9,000 format strings), each conversion only with the flags it defines (no `+`/space on unsigned, `#` only on `x`/`X`, no precision on `%c`/`%p`), so a mismatch is always a real bug. `%c` includes `'\0'` with padding.

### Memory Leak Tests

Dedicated leak tests using **Valgrind** (any leak, invalid access or use of uninitialised memory fails):
- Short prints
- Long prints (8 KB), to cover implementations that switch to `malloc` past some buffer size
- Many arguments
- NULL pointers (`%s` with NULL, `%p` with NULL)

For bonus:
- Combined flags on short strings
- Precision truncation
- Precision = 0
- Huge width padding (5000+ characters)
- Huge strings with width

## 📁 Project Structure

```
printf-fairy/
├── run.sh                    # Main test orchestrator
├── basic_tests.c             # Mandatory conversion tests
├── basic_tests_bonus.c       # Bonus flag/width/precision tests
├── leak_tests.c              # Leak tests (mandatory paths)
├── leak_tests_bonus.c        # Leak tests (bonus paths)
├── utils.c                   # Capture, diff, forked runner, malloc wrapper
├── printf_fairy.h            # CHECK() / SWEEP() macros, shared types
└── README.md                 # This file
```

## 🔍 Advanced Features

### Direct comparison with libc printf

`CHECK(fmt, ...)` runs your real `ft_printf` and the libc `printf` with the same arguments, captures what each one writes to stdout through a pipe, and compares the raw bytes (not C strings, so a `'\0'` printed by `%c` cannot hide the rest of the output) **and** the return values.

### Forked tests

Every test runs in its own process with a 5 second alarm, so a segfault or an infinite loop only fails that test, with the signal or the timeout reported under it.

### Adding a test

Write a function made of `CHECK()` calls and add it to its section's `t_test` table:

```c
static void	d_zero_test(void) {
	CHECK("[%d]", 0);
}

static const t_test	g_d_i_tests[] = {
	{"d zero", d_zero_test},
	...
};
```

### Mandatory tests re-run on the bonus build

Many bonus implementations rewrite every conversion to support flags, so the mandatory conversion tests and leak tests are compiled and run a second time against the bonus library. A bug that only exists in the bonus code (e.g. `%c` with `'\0'`) is caught even when the mandatory build is green.

### Malloc wrapper

`-Wl,--wrap=malloc` routes your library's `malloc` calls through `__wrap_malloc`, which can make the Nth call return `NULL` (used by the malloc failure test).

## 🐛 Debugging

If a test fails:

1. **Failure details**: each `[KO]` shows the format, the expected output and yours
2. **Verbose mode**: Run `./run.sh -v` to also list passing tests
3. **Valgrind logs**: Complete output displayed for memory issues
4. **Norminette**: Specific errors shown with file and line
5. **Build errors**: Printed on failure

## ⚠️ Limitations

- Tested only on Linux (uses `fork`, `nm`, Valgrind, and `--wrap` linker support)
- Requires specific directory structure (tester in ft_printf subfolder)
- The reference is glibc's `printf` (e.g. `(nil)` for a NULL `%p`), as on 42's Linux machines
- `memset`/`memcpy` must be allowed because compilers emit them on their own, so a direct call to them is not detected
- The `__attribute__((format))` in your `ft_printf.h` may emit warnings on intentionally invalid format strings; the tester uses `-Wno-format` to bypass this

## 🤝 Contributing

Contributions are welcome! Feel free to:

- Open an issue to report a bug
- Suggest new edge cases
- Improve test coverage
- Enhance documentation

## 📜 License

This project is free to use for educational purposes.

## 💖 Credits

Created with ✨ by **sys-harmony**.

---

*If printf-fairy helped you validate your ft_printf, don't forget to leave a ⭐ on the repo!*
