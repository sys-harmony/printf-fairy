#ifndef PRINTF_FAIRY_H
# define PRINTF_FAIRY_H

// =============================
// 📚 Libraries
// =============================
# include <ctype.h>
# include <limits.h>
# include <signal.h>
# include <stdarg.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/wait.h>

// =============================
// 🎨 Global define(s)
// =============================
# define GREEN			"\033[32m"
# define RED			"\033[31m"
# define YELLOW			"\033[33m"
# define BOLD			"\033[1m"
# define DIM			"\033[2m"
# define RESET			"\033[0m"
# define WIDTH			46		// as in run.sh
# define TEST_TIMEOUT	5		// seconds before a test counts as hanging
# ifndef VERBOSE
#  define VERBOSE		0
# endif

# define CAPTURE_SIZE	16384

// =============================
// 🧱 Type(s)
// =============================
typedef struct s_test
{
	const char	*name;
	void		(*fn)(void);
}	t_test;

typedef struct s_capture
{
	char	buf[CAPTURE_SIZE];
	ssize_t	len;
	int		ret;
}	t_capture;

// =============================
// 🌍 Global variable(s)
// =============================
extern int	g_malloc_wrap_enabled;
extern int	g_malloc_count;
extern int	g_malloc_fail_at;

// =============================
// 🪄 Macro(s)
// =============================
# define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))
# define RUN_GROUP(title, tests) run_group(title, tests, ARRAY_SIZE(tests))

/*
 * Runs ft_printf, then printf, with the same arguments and compares the raw
 * bytes they write and their return values. CHECK fails on a difference.
 * CHECK_UB is for what the C standard leaves undefined (flag 0 on %s, a
 * precision on %p...): glibc is then only a reference, a difference warns.
 */
# define COMPARE(ub, fmt, ...) \
	do { \
		t_capture	ft_; \
		t_capture	libc_; \
		capture_start(); \
		ft_.ret = ft_printf(fmt, ##__VA_ARGS__); \
		capture_end(&ft_); \
		capture_start(); \
		libc_.ret = printf(fmt, ##__VA_ARGS__); \
		capture_end(&libc_); \
		compare(ub, fmt, &ft_, &libc_); \
	} while (0)
# define CHECK(fmt, ...)	COMPARE(0, fmt, ##__VA_ARGS__)
# define CHECK_UB(fmt, ...)	COMPARE(1, fmt, ##__VA_ARGS__)

/*
 * Compares every "[%<flags><width><precision><conversion>]" built from the
 * lists, against each value: tens of thousands of formats in a few lines.
 * defined() sorts them: what the standard defines must match glibc, the
 * rest (flag 0 on %s, + on %u, a precision on %c...) only warns.
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
		COMPARE(!defined(flags[f_], *precs[p_], *c_), fmt_, vals[v_]); \
	}

// =============================
// 📋 Function prototype(s)
// =============================
int			ft_printf(const char *format, ...);

void		capture_start(void);
void		capture_end(t_capture *cap);
void		compare(int ub, const char *fmt, const t_capture *ft,
				const t_capture *libc);
int			defined(const char *flags, int precision, char conv);
void		silence_stdout(void);
void		*__real_malloc(size_t size);
void		*__wrap_malloc(size_t size);

void		die(const char *what) __attribute__((noreturn));
void		fail(const char *fmt, ...)
			__attribute__((noreturn, format(printf, 1, 2)));
void		warning(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void		section(const char *title);
void		run_group(const char *title, const t_test *tests, size_t n);
int			fairy_summary(void);

#endif //PRINTF_FAIRY_H
