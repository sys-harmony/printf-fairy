#ifndef PRINTF_FAIRY_H
# define PRINTF_FAIRY_H

// =============================
// 📚 Libraries
// =============================
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <stdarg.h>
# include <stddef.h>
# include <stdlib.h>

// =============================
// 🎨 Global define(s)
// =============================
# define GREEN			"\033[0;32m"
# define RED			"\033[0;31m"
# define RESET			"\033[0m"

# ifndef VERBOSE
#  define VERBOSE 0
# endif

# define CAPTURE_SIZE	4096
# define FAIL_MSG_SIZE	512
# define TIMEOUT_SEC	5

// =============================
// 🧱 Type(s)
// =============================
typedef struct s_capture
{
	char	buf[CAPTURE_SIZE];
	ssize_t	len;
	int		ret;
}	t_capture;

typedef struct s_test
{
	const char	*name;
	void		(*fn)(void);
}	t_test;

// =============================
// 🌍 Global variable(s)
// =============================
extern int	g_malloc_wrap_enabled;
extern int	g_malloc_count;
extern int	g_malloc_fail_at;
extern int	g_tests_failed;

// =============================
// 🪄 Macro(s)
// =============================
# define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))
# define RUN_SECTION(title, tests) run_section(title, tests, ARRAY_SIZE(tests))

/*
 * Runs ft_printf then printf with the same arguments, capturing the raw bytes
 * written to stdout. On any difference (bytes or return value) the diff is
 * recorded for the parent process and the forked test aborts.
 */
# define CHECK(fmt, ...) \
	do { \
		t_capture	ft_; \
		t_capture	libc_; \
		capture_start(); \
		ft_.ret = ft_printf(fmt, ##__VA_ARGS__); \
		capture_end(&ft_); \
		capture_start(); \
		libc_.ret = printf(fmt, ##__VA_ARGS__); \
		capture_end(&libc_); \
		check_output(fmt, &ft_, &libc_); \
	} while (0)

/*
 * CHECKs every "[%<flag><width><prec><conv>]" combination against each value.
 * Each caller passes only the flags defined for its conversions, so a
 * mismatch is always a real bug and never undefined behaviour.
 */
# define SWEEP(convs, flags, widths, precs, vals) \
	for (size_t f_ = 0; f_ < ARRAY_SIZE(flags); f_++) \
	for (size_t w_ = 0; w_ < ARRAY_SIZE(widths); w_++) \
	for (size_t p_ = 0; p_ < ARRAY_SIZE(precs); p_++) \
	for (const char *c_ = convs; *c_; c_++) \
	for (size_t v_ = 0; v_ < ARRAY_SIZE(vals); v_++) \
	{ \
		char	fmt_[32]; \
		snprintf(fmt_, sizeof(fmt_), "[%%%s%s%s%c]", \
			flags[f_], widths[w_], precs[p_], *c_); \
		CHECK(fmt_, vals[v_]); \
	}

// =============================
// 📋 Function prototype(s)
// =============================
void		capture_start(void);
void		capture_end(t_capture *cap);
void		check_output(const char *fmt, const t_capture *ft,
				const t_capture *libc);
void		test_fail(const char *fmt, ...)
			__attribute__((noreturn, format(printf, 1, 2)));
void		silence_stdout(void);
void		run_section(const char *title, const t_test *tests, size_t n);
void		error_exit(char *msg);
void		*__real_malloc(size_t size);
void		*__wrap_malloc(size_t size);

#endif //PRINTF_FAIRY_H
