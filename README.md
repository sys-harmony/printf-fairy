# 🧚 printf-fairy

A comprehensive tester for 42's **ft_printf** project, featuring memory leak detection, edge case testing, and rigorous validation against the original `printf`.

## ✨ Features

- ✅ Complete tests for all conversions (mandatory + bonus)
- 🧹 Clean execution (automatic cleanup of temporary files)
- 🔍 Automatic verification with **norminette**
- 📋 Prototype validation in project headers
- ⚙️ Makefile rules verification (NAME, all, clean, fclean, re, bonus)
- 🔧 Compilation flags check (-Wall -Wextra -Werror)
- 🔎 External function usage verification
- 💧 Memory leak detection with **Valgrind**
- 🚨 Edge case testing (NULL, INT_MIN, UINT_MAX, null bytes, huge widths)
- 📊 Clear and colorful output
- 🔇 Optional verbose mode for detailed results

## 📋 Prerequisites

Before using printf-fairy, make sure you have installed:

- `gcc` (with `--wrap` linker support)
- `make`
- `valgrind`
- `norminette`
- `bash`
- `nm` (usually pre-installed)

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
╔═══════════════════════════╗
║      PRINTF-FAIRY 🧚      ║
╚═══════════════════════════╝

📝 Checking norm...         Done
🔖 Checking version...     Bonus
📋 Checking prototype...    Done
⚙️  Checking Makefile...    Done
📦 Building ft_printf...    Done
🔍 Checking externals...    Done
🔨 Building tests...        Done
🧪 Running tests...

========================================
ft_printf %d / %i
========================================
[OK]  Test d zero
[OK]  Test d positive
[OK]  Test d negative
[OK]  Test d INT_MAX
[OK]  Test d INT_MIN
[OK]  Test i basic
...

╔═══════════════════════════╗
║     OH MY, YOU PASSED!    ║
╚═══════════════════════════╝
```

On failure, details of failed tests will be displayed, along with Valgrind logs if memory leaks are detected.

## 🧪 Tests Covered

### Pre-compilation Checks

- **Norminette**: Validates all .c and .h files
- **Prototype**: Verifies `int ft_printf(const char *, ...)` in any project header (`*.h`)
- **Makefile**: Checks for required rules and proper compilation flags
- **External functions**: Ensures only allowed symbols are referenced (`malloc`, `free`, `write`, plus `memset`/`memcpy` which gcc may emit from struct initialisation; `va_*` are inlined and invisible to `nm`)

### Mandatory Conversions

#### %c (character)
- Basic ASCII
- Null byte (`'\0'`)
- Low and high ASCII range

#### %s (string)
- Basic, empty, NULL
- Long strings

#### %p (pointer)
- NULL, small addresses, large addresses, stack addresses

#### %d / %i (signed decimal)
- Zero, positive, negative
- `INT_MAX`, `INT_MIN`

#### %u (unsigned decimal)
- Zero, positive, `UINT_MAX`

#### %x / %X (hexadecimal)
- Zero, basic values, `UINT_MAX`
- Mixed lower/upper case

#### %% (percent sign)
- Basic, with text, consecutive

#### Mix
- Multiple conversions in one call
- All types in one format

#### Return value (broken fd)
- Empty output, empty `%s`, and text on a closed stdout: `ft_printf` must return `-1` like glibc (a zero-length write on an invalid fd still fails)
- Normal output returns the exact character count

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

#### Flag ` ` (space)
- With positive, negative, zero
- Correctly overridden by `+`

#### Flag `+` (force sign)
- With positive, negative, zero
- With zero-padding

#### Width
- Basic, too small (ignored), width 0
- With strings

#### Precision
- Basic for `%d`
- Precision 0 with n=0 (empty output)
- String truncation
- Precision longer than string
- Width + precision combinations

#### Combinations
- All flags + width + precision
- `#` + `0` + width with `%x`
- `-` + width + precision with `%s`
- Repeated flags
- Mixed format strings

#### Negative as unsigned
- `-1` and `INT_MIN` passed to `%u`, `%x`, `%X` (must be reinterpreted as unsigned)
- With width, precision and `#`

#### Misc flags
- `%p` with width (`%20p`, `%-20p`) and zero-padding (`%010p`)
- Flag `0` correctly ignored for `%s`
- Huge width padding (1000+ characters)

#### Exhaustive Cartesian Sweeps
- Massive combinations of format strings testing flags, widths, and precisions against various values for signed, unsigned, hexadecimal, and string conversions.

#### Isolated Edge Cases
- Huge precision limits (up to `.40`)
- Edge cases for `#` with precision 0 and 5.
- Extreme pointer values like `-1`, `0x1`, and `0xdeadbeef`

### Memory Leak Tests

Dedicated leak tests using **Valgrind**:
- Short prints (stack buffer path)
- Long prints (heap allocation path in `ft_vdprintf`)
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
├── utils.c                   # Helpers + malloc wrapper + COMPARE()
├── printf_fairy.h            # Header file
└── README.md                 # This file
```

## 🔍 Advanced Features

### Direct comparison with libc printf

Each test runs your real `ft_printf` and the libc `printf` on the same format, capturing both outputs through a pipe (`capture_start`/`capture_end`) and comparing the captured strings **and** the return values. This validates the full call chain (`ft_printf` -> `ft_vprintf` -> `ft_vdprintf` -> `ft_vsnprintf`) exactly as it runs in production, not just the buffer formatter.

### Forked tests

Crash-prone tests (`%s` with NULL, edge cases) run in separate processes to prevent tester crashes, allowing comprehensive validation without interruption.

### Mandatory tests re-run on the bonus build

Because the bonus `ft_vsnprintf_bonus.c` re-implements **every** conversion (not just the flags), the mandatory conversion tests and leak tests are compiled and run a second time against the bonus library. A bug that only exists in the bonus conversion code (e.g. `%c` with `'\0'`, a padding overflow) is caught while the mandatory build stays green.

### Stack and heap path coverage

`ft_vdprintf` uses a stack buffer for short outputs and allocates on the heap when the output exceeds `PRINTF_BUFFER_SIZE`. Leak tests intentionally trigger both paths.

### Malloc wrapper

A custom malloc wrapper (`__wrap_malloc`) is available via the `-Wl,--wrap=malloc` linker flag for malloc failure simulation in future test extensions.

## 🐛 Debugging

If a test fails:

1. **Verbose mode**: Run `./run.sh -v` to see all test details
2. **Valgrind logs**: Complete output displayed for memory issues
3. **Norminette**: Specific errors shown with file and line
4. **Build errors**: Captured per target under the run's temporary directory and printed on failure

## ⚠️ Limitations

- Tested only on Linux (uses `fork`, `nm`, Valgrind, and `--wrap` linker support)
- Requires specific directory structure (tester in ft_printf subfolder)
- Comparison uses `vsnprintf` from libc as reference — assumes glibc behavior
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
