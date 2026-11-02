#include "printf_fairy.h"

/* ************************************************************************** */
/*                     flags, one classic pitfall per test                    */
/* ************************************************************************** */

static void	minus_test(void) {
	CHECK("[%-5c]", 'A');
	CHECK("[%-5c]", '\0');
	CHECK("[%-10s]", "hi");
	CHECK("[%-10d] [%-10d]", 42, -42);
	CHECK("[%-10u] [%-10x] [%-10X]", 42u, 255u, 255u);
	CHECK("[%-20p] [%-20p]", (void *)0x42, NULL);
}

static void	zero_test(void) {
	CHECK("[%010d] [%010d] [%010i]", 42, -42, INT_MIN);
	CHECK("[%08x] [%08X] [%08u]", 255u, 255u, 255u);
	CHECK("[%0d] [%01d] [%02d]", 42, -42, -4);
}

static void	zero_ignored_test(void) {
	CHECK("[%-010d]", 42);
	CHECK("[%010.5d] [%010.5d]", 42, -42);
	CHECK("[%05.0d] [%05.2d] [%05.1d]", 0, 0, 42);
	CHECK("[%08.2x] [%08.0X]", 255u, 0u);
}

static void	hash_test(void) {
	CHECK("[%#x] [%#X]", 255u, 255u);
	CHECK("[%#x] [%#X]", 0u, 0u);
	CHECK("[%#10x] [%#-10x] [%#010x] [%#010X]", 255u, 255u, 255u, 255u);
	CHECK("[%#.5x] [%#.0x] [%#.0x] [%#.5X]", 255u, 0u, 1u, 255u);
	CHECK("[%#x]", -1);
}

static void	sign_test(void) {
	CHECK("[%+d] [%+d] [%+d] [%+i]", 42, -42, 0, INT_MIN);
	CHECK("[% d] [% d] [% d] [% i]", 42, -42, 0, INT_MAX);
	CHECK("[%+ d] [% +d] [%+010d] [% 010d]", 42, 42, 42, 42);
	CHECK("[%+.0d] [% .0d] [%+5.3d] [% -6d]", 0, 0, 7, 7);
}

static void	width_test(void) {
	CHECK("[%10d] [%2d] [%1d]", 42, 42424, -1);
	CHECK("[%10s] [%10s] [%2s]", "hi", (char *)NULL, "hello");
	CHECK("[%5c] [%5c]", 'x', '\0');
	CHECK("[%20p] [%20p]", (void *)0x42, NULL);
	CHECK("[%1000d]", 42);
	CHECK("[%-1000s]", "x");
}

static void	precision_test(void) {
	CHECK("[%.5d] [%.5d] [%.5i] [%.40u]", 42, -42, INT_MIN, 7u);
	CHECK("[%.0d] [%.0d] [%.d] [%.d]", 0, 42, 0, 42);
	CHECK("[%.0u] [%.0x] [%.0X]", 0u, 0u, 0u);
	CHECK("[%.3s] [%.0s] [%.s] [%.10s]", "hello", "hello", "hello", "hi");
	CHECK("[%.6s] [%.10s]", (char *)NULL, (char *)NULL);
	CHECK("[%10.5d] [%-10.5d] [%10.3s] [%-10.3s]", 42, -42, "hello", "hello");
	CHECK("[%15.12u] [%15.12x]", -1, -1);
}

static void	combo_test(void) {
	CHECK("[%+010.5d] [%-+10.5d] [% -10.5d]", 42, 42, -42);
	CHECK("[%5.3s] [%+05d] [%#010x] [%-5c] [%%]", "hello", -42, 255u, 'z');
	CHECK("[%-#12.8x] [%#-12.8X]", 0xbeefu, 0xbeefu);
}

static const t_test	g_flag_tests[] = {
	{"-: left-justifies every conversion", minus_test},
	{"0: pads numbers with zeros, after the sign", zero_test},
	{"0 ignored with - or with a precision", zero_ignored_test},
	{"#: 0x prefix, none for 0, with width and 0", hash_test},
	{"+ and space: signs of %d %i, + wins", sign_test},
	{"width: padding, too small, 1000 wide", width_test},
	{"precision: digits, truncation, .0 on 0", precision_test},
	{"flags, width and precision together", combo_test},
};

/* ************************************************************************** */
/*          undefined combinations: glibc as reference, warnings only         */
/* ************************************************************************** */

static void	ub_zero_test(void) {
	CHECK_UB("[%010p] [%-010p] [%010p]", (void *)0x42, (void *)0x42, NULL);
	CHECK_UB("[%05s] [%05c]", "hi", 'a');
}

static void	ub_precision_test(void) {
	CHECK_UB("[%.5p] [%.0p] [%.3c]", (void *)0x42, NULL, 'b');
	CHECK_UB("[%.3s] [%10.2s]", (char *)NULL, (char *)NULL);
}

static void	ub_sign_hash_test(void) {
	CHECK_UB("[%+s] [% s] [%+c] [%#c] [%#s]", "s", "s", 'c', 'c', "s");
	CHECK_UB("[%+u] [% x] [%#d] [%#i] [%#u]", 42u, 42u, 42, 42, 42u);
	CHECK_UB("[%+p] [% p] [%#p]", (void *)1, (void *)1, (void *)1);
}

static void	ub_repeat_test(void) {
	CHECK_UB("[%+++d] [%---5d] [%  d] [%##x] [%000005d]", 42, 42, 42, 42u, 42);
	CHECK_UB("[%5%] [%-5%] [%05%]");
}

static const t_test	g_ub_tests[] = {
	{"flag 0 on %p, %s, %c", ub_zero_test},
	{"precision on %p, %c, NULL %s", ub_precision_test},
	{"+, space and # where undefined", ub_sign_hash_test},
	{"repeated flags, flags on %%", ub_repeat_test},
};

/* ************************************************************************** */
/*                            exhaustive sweeps                               */
/* ************************************************************************** */

/*
 * Every flag combination x width x precision x value, for each conversion:
 * about 22,000 formats. What the C standard defines must match glibc, the
 * undefined combinations only warn (see defined()).
 */

static const char	*g_flags[] = {"", "-", "0", "+", " ", "#", "-0", "-+", "- ",
	"-#", "+0", " 0", "#0", "+ ", "-+0#"};
static const char	*g_widths[] = {"", "1", "5", "10", "20"};
static const char	*g_precs[] = {"", ".", ".0", ".1", ".2", ".5", ".10"};

static void	sweep_di_test(void) {
	const int	vals[] = {0, 1, -1, 7, 42, -42, INT_MAX, INT_MIN};

	SWEEP("di", g_flags, g_widths, g_precs, vals);
}

static void	sweep_u_test(void) {
	const unsigned	vals[] = {0, 1, 42, 4242, UINT_MAX, 2147483648u};

	SWEEP("u", g_flags, g_widths, g_precs, vals);
}

static void	sweep_x_test(void) {
	const unsigned	vals[] = {0, 1, 255, 0xabcdef, UINT_MAX, 2147483648u};

	SWEEP("xX", g_flags, g_widths, g_precs, vals);
}

static void	sweep_c_test(void) {
	const int	vals[] = {'a', '\0', 127};

	SWEEP("c", g_flags, g_widths, g_precs, vals);
}

static void	sweep_s_test(void) {
	const char	*precs[] = {"", ".", ".0", ".3", ".10"};
	const char	*vals[] = {"", "a", "hello", "abcdefghijklmnop"};

	SWEEP("s", g_flags, g_widths, precs, vals);
}

static void	sweep_p_test(void) {
	void *const	vals[] = {NULL, (void *)0x42, (void *)-1};

	SWEEP("p", g_flags, g_widths, g_precs, vals);
}

static const t_test	g_sweep_tests[] = {
	{"%d %i: 8,400 formats", sweep_di_test},
	{"%u: 3,150 formats", sweep_u_test},
	{"%x %X: 6,300 formats", sweep_x_test},
	{"%c: 1,575 formats", sweep_c_test},
	{"%s: 1,500 formats", sweep_s_test},
	{"%p: 1,575 formats", sweep_p_test},
};

int	main(void) {
	RUN_GROUP("ft_printf flags (bonus)", g_flag_tests);
	RUN_GROUP("ft_printf undefined combinations (bonus)", g_ub_tests);
	RUN_GROUP("ft_printf exhaustive sweeps (bonus)", g_sweep_tests);
	return fairy_summary();
}
