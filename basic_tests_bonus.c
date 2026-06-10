#include "printf_fairy.h"
#include <limits.h>
#include <stdlib.h>

#ifndef VERBOSE
# define VERBOSE 0
#endif

/* ************************************************************************** */
/*                              flag minus (-)                                */
/* ************************************************************************** */

static void	minus_c_test(void) {
	if (!COMPARE("[%-5c]", 'A'))
		abort();
}

static void	minus_s_test(void) {
	if (!COMPARE("[%-10s]", "hi"))
		abort();
}

static void	minus_d_test(void) {
	if (!COMPARE("[%-10d]", 42) || !COMPARE("[%-10d]", -42))
		abort();
}

static void	minus_x_test(void) {
	if (!COMPARE("[%-10x]", 255))
		abort();
}

static void	test_flag_minus(void) {
	const char		*tests[] = {
		"with %c",
		"with %s",
		"with %d",
		"with %x"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	const int		passed[] = {
		!forked_test(minus_c_test),
		!forked_test(minus_s_test),
		!forked_test(minus_d_test),
		!forked_test(minus_x_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("flag - (bonus)", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                              flag zero (0)                                 */
/* ************************************************************************** */

static void	zero_d_test(void) {
	if (!COMPARE("[%010d]", 42))
		abort();
}

static void	zero_d_negative_test(void) {
	if (!COMPARE("[%010d]", -42))
		abort();
}

static void	zero_x_test(void) {
	if (!COMPARE("[%08x]", 255))
		abort();
}

static void	zero_minus_test(void) {
	if (!COMPARE("[%-010d]", 42))
		abort();
}

static void	zero_precision_test(void) {
	if (!COMPARE("[%010.5d]", 42))
		abort();
}

static void	zero_precision_zero_value_test(void) {
	if (!COMPARE("[%05.0d]", 0) || !COMPARE("[%05.2d]", 0))
		abort();
}

static void	zero_precision_short_test(void) {
	if (!COMPARE("[%05.1d]", 42) || !COMPARE("[%08.2x]", 255))
		abort();
}

static void	test_flag_zero(void) {
	const char		*tests[] = {
		"basic",
		"with negative",
		"with %x",
		"ignored if -",
		"ignored if precision",
		"ignored if precision (zero value)",
		"ignored if precision (prec <= digits)"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	const int		passed[] = {
		!forked_test(zero_d_test),
		!forked_test(zero_d_negative_test),
		!forked_test(zero_x_test),
		!forked_test(zero_minus_test),
		!forked_test(zero_precision_test),
		!forked_test(zero_precision_zero_value_test),
		!forked_test(zero_precision_short_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("flag 0 (bonus)", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                              flag hash (#)                                 */
/* ************************************************************************** */

static void	hash_x_test(void) {
	if (!COMPARE("[%#x]", 255) || !COMPARE("[%#X]", 255))
		abort();
}

static void	hash_zero_test(void) {
	if (!COMPARE("[%#x]", 0) || !COMPARE("[%#X]", 0))
		abort();
}

static void	hash_width_test(void) {
	if (!COMPARE("[%#10x]", 255))
		abort();
}

static void	hash_zero_pad_test(void) {
	if (!COMPARE("[%#010x]", 255))
		abort();
}

static void	test_flag_hash(void) {
	const char		*tests[] = {
		"basic",
		"with zero (no prefix)",
		"with width",
		"with zero pad"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	const int		passed[] = {
		!forked_test(hash_x_test),
		!forked_test(hash_zero_test),
		!forked_test(hash_width_test),
		!forked_test(hash_zero_pad_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("flag # (bonus)", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                              flag space ( )                                */
/* ************************************************************************** */

static void	space_positive_test(void) {
	if (!COMPARE("[% d]", 42))
		abort();
}

static void	space_negative_test(void) {
	if (!COMPARE("[% d]", -42))
		abort();
}

static void	space_zero_test(void) {
	if (!COMPARE("[% d]", 0))
		abort();
}

static void	space_overridden_by_plus_test(void) {
	if (!COMPARE("[%+ d]", 42) || !COMPARE("[% +d]", 42))
		abort();
}

static void	test_flag_space(void) {
	const char		*tests[] = {
		"with positive",
		"with negative",
		"with zero",
		"overridden by +"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	const int		passed[] = {
		!forked_test(space_positive_test),
		!forked_test(space_negative_test),
		!forked_test(space_zero_test),
		!forked_test(space_overridden_by_plus_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("flag space (bonus)", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                              flag plus (+)                                 */
/* ************************************************************************** */

static void	plus_positive_test(void) {
	if (!COMPARE("[%+d]", 42))
		abort();
}

static void	plus_negative_test(void) {
	if (!COMPARE("[%+d]", -42))
		abort();
}

static void	plus_zero_test(void) {
	if (!COMPARE("[%+d]", 0))
		abort();
}

static void	plus_zero_pad_test(void) {
	if (!COMPARE("[%+010d]", 42))
		abort();
}

static void	test_flag_plus(void) {
	const char		*tests[] = {
		"with positive",
		"with negative",
		"with zero",
		"with zero pad"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	const int		passed[] = {
		!forked_test(plus_positive_test),
		!forked_test(plus_negative_test),
		!forked_test(plus_zero_test),
		!forked_test(plus_zero_pad_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("flag + (bonus)", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                                  width                                     */
/* ************************************************************************** */

static void	width_basic_test(void) {
	if (!COMPARE("[%10d]", 42))
		abort();
}

static void	width_too_small_test(void) {
	if (!COMPARE("[%2d]", 42424))
		abort();
}

static void	width_zero_test(void) {
	if (!COMPARE("[%0d]", 42))
		abort();
}

static void	width_with_string_test(void) {
	if (!COMPARE("[%10s]", "hi"))
		abort();
}

static void	test_width(void) {
	const char		*tests[] = {
		"basic",
		"too small (ignored)",
		"width 0",
		"with string"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	const int		passed[] = {
		!forked_test(width_basic_test),
		!forked_test(width_too_small_test),
		!forked_test(width_zero_test),
		!forked_test(width_with_string_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("width (bonus)", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                                precision                                   */
/* ************************************************************************** */

static void	prec_d_basic_test(void) {
	if (!COMPARE("[%.5d]", 42))
		abort();
}

static void	prec_d_zero_test(void) {
	if (!COMPARE("[%.0d]", 0) || !COMPARE("[%.0d]", 42))
		abort();
}

static void	prec_s_truncate_test(void) {
	if (!COMPARE("[%.3s]", "hello") || !COMPARE("[%.0s]", "hello"))
		abort();
}

static void	prec_s_longer_test(void) {
	if (!COMPARE("[%.10s]", "hi"))
		abort();
}

static void	prec_x_test(void) {
	if (!COMPARE("[%.5x]", 255) || !COMPARE("[%.0x]", 0))
		abort();
}

static void	prec_no_number_test(void) {
	if (!COMPARE("[%.d]", 42) || !COMPARE("[%.d]", 0))
		abort();
}

static void	prec_width_combination_test(void) {
	if (!COMPARE("[%10.5d]", 42) || !COMPARE("[%10.3s]", "hello"))
		abort();
}

static void	test_precision(void) {
	const char		*tests[] = {
		"d basic",
		"d precision 0",
		"s truncation",
		"s precision > len",
		"x basic",
		"precision no number",
		"width + precision"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	const int		passed[] = {
		!forked_test(prec_d_basic_test),
		!forked_test(prec_d_zero_test),
		!forked_test(prec_s_truncate_test),
		!forked_test(prec_s_longer_test),
		!forked_test(prec_x_test),
		!forked_test(prec_no_number_test),
		!forked_test(prec_width_combination_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("precision (bonus)", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                            combinations                                    */
/* ************************************************************************** */

static void	combo_all_flags_test(void) {
	if (!COMPARE("[%+010.5d]", 42))
		abort();
}

static void	combo_hash_zero_pad_x_test(void) {
	if (!COMPARE("[%#010x]", 255) || !COMPARE("[%#010X]", 255))
		abort();
}

static void	combo_minus_width_prec_s_test(void) {
	if (!COMPARE("[%-10.3s]", "hello"))
		abort();
}

static void	combo_repeated_flags_test(void) {
	if (!COMPARE("[%+++d]", 42))
		abort();
}

static void	combo_mixed_test(void) {
	if (!COMPARE("[%5.3s] [%+05d] [%#010x]", "hello", -42, 255))
		abort();
}

static void	test_combinations(void) {
	const char		*tests[] = {
		"all flags + width + precision",
		"# 0 width with %x",
		"- width precision with %s",
		"repeated flags",
		"mixed format"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	const int		passed[] = {
		!forked_test(combo_all_flags_test),
		!forked_test(combo_hash_zero_pad_x_test),
		!forked_test(combo_minus_width_prec_s_test),
		!forked_test(combo_repeated_flags_test),
		!forked_test(combo_mixed_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("combinations (bonus)", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                        flags on %p / %s / huge width                       */
/* ************************************************************************** */

/*
 * Edge cases rarely tested elsewhere: width and 0 flag on %p, 0 flag ignored on %s
 * (0 only applies to numeric conversions), and a huge width to test padding
 * logic without memory risks.
 * The simple negative case (%u/%x/%X with -1, INT_MIN) is already covered by
 * mandatory tests, which are re-run on the bonus lib; we only keep the
 * negative + width/precision/hash combination here, specific to the bonus.
 */

static void	neg_unsigned_with_flags_test(void) {
	if (!COMPARE("[%15.12u]", -1) || !COMPARE("[%#x]", -1))
		abort();
}

static void	pointer_width_test(void) {
	if (!COMPARE("[%20p]", (void *)0x42)
		|| !COMPARE("[%-20p]", (void *)0x42))
		abort();
}

static void	pointer_zero_pad_test(void) {
	if (!COMPARE("[%010p]", (void *)0x42))
		abort();
}

static void	string_zero_flag_ignored_test(void) {
	if (!COMPARE("[%05s]", "hi"))
		abort();
}

static void	huge_width_test(void) {
	if (!COMPARE("[%1000d]", 42) || !COMPARE("[%-1000s]", "x"))
		abort();
}

static void	test_misc_flags(void) {
	const char		*tests[] = {
		"negative unsigned + width/precision/hash",
		"%p with width (- and right)",
		"%p with zero pad",
		"%s ignores flag 0",
		"huge width padding"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	const int		passed[] = {
		!forked_test(neg_unsigned_with_flags_test),
		!forked_test(pointer_width_test),
		!forked_test(pointer_zero_pad_test),
		!forked_test(string_zero_flag_ignored_test),
		!forked_test(huge_width_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("misc flags (bonus)", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                    exhaustive cartesian product (bonus)                    */
/* ************************************************************************** */

/*
 * Inspired by francinette's Fsoares: instead of isolated cases, sweep the full
 * flag x width x precision x value space for each conversion and compare every
 * combination against glibc. The per-conversion flag sets avoid undefined
 * behaviour (e.g. + / space are never applied to unsigned conversions, # only
 * to x/X), so a mismatch always means a real bug. The whole sweep runs inside
 * a single forked process; abort() fires on the first divergence.
 */

static void	sweep(const char *convs[], const char *flags[], size_t nflags,
				const int vals[], size_t nvals) {
	const char	*widths[] = {"", "1", "5", "10"};
	const char	*precs[] = {"", ".", ".0", ".1", ".2", ".5", ".10"};
	char		fmt[32];

	for (size_t c = 0; convs[c]; c++)
		for (size_t f = 0; f < nflags; f++)
			for (size_t w = 0; w < ARRAY_SIZE(widths); w++)
				for (size_t p = 0; p < ARRAY_SIZE(precs); p++)
					for (size_t v = 0; v < nvals; v++)
					{
						snprintf(fmt, sizeof(fmt), "[%%%s%s%s%s]",
							flags[f], widths[w], precs[p], convs[c]);
						if (!COMPARE(fmt, vals[v]))
						{
							fail_record_int(fmt, vals[v]);
							abort();
						}
					}
}

static void	exhaustive_signed_test(void) {
	const char	*convs[] = {"d", "i", NULL};
	const char	*flags[] = {"", "-", "0", "+", " ", "-0", "+0", " 0", "-+"};
	const int	vals[] = {0, 1, -1, 42, -42, 2147483647, -2147483648, 7};

	sweep(convs, flags, ARRAY_SIZE(flags), vals, ARRAY_SIZE(vals));
}

static void	exhaustive_unsigned_test(void) {
	const char	*convs[] = {"u", NULL};
	const char	*flags[] = {"", "-", "0", "-0"};
	const int	vals[] = {0, 1, 42, -1, -2147483648, 4242};

	sweep(convs, flags, ARRAY_SIZE(flags), vals, ARRAY_SIZE(vals));
}

static void	exhaustive_hex_test(void) {
	const char	*convs[] = {"x", "X", NULL};
	const char	*flags[] = {"", "-", "0", "#", "-0", "0#", "-#"};
	const int	vals[] = {0, 1, 255, -1, -2147483648, 0xabcdef};

	sweep(convs, flags, ARRAY_SIZE(flags), vals, ARRAY_SIZE(vals));
}

static void	exhaustive_str_test(void) {
	const char	*flags[] = {"", "-"};
	const char	*widths[] = {"", "1", "5", "10"};
	const char	*precs[] = {"", ".", ".0", ".3", ".10"};
	const char	*vals[] = {"", "a", "hello", "abcdefghijklmnop"};
	char		fmt[32];

	for (size_t f = 0; f < ARRAY_SIZE(flags); f++)
		for (size_t w = 0; w < ARRAY_SIZE(widths); w++)
			for (size_t p = 0; p < ARRAY_SIZE(precs); p++)
				for (size_t v = 0; v < ARRAY_SIZE(vals); v++)
				{
					snprintf(fmt, sizeof(fmt), "[%%%s%s%ss]",
						flags[f], widths[w], precs[p]);
					if (!COMPARE(fmt, vals[v]))
					{
						fail_record_str(fmt, vals[v]);
						abort();
					}
				}
}

static int	run_sweep(void (*fn)(void), char *msg) {
	int	passed;

	passed = !forked_test(fn);
	msg[0] = '\0';
	if (!passed && g_fail_msg && g_fail_msg[0])
		strncpy(msg, g_fail_msg, 255);
	if (g_fail_msg)
		g_fail_msg[0] = '\0';
	return (passed);
}

static const char	**build_labels(const char *tests[],
						char msgs[][256], size_t num_tests) {
	static char			buf[4][320];
	static const char	*labels[4];

	for (size_t i = 0; i < num_tests; i++)
	{
		if (msgs[i][0])
			snprintf(buf[i], sizeof(*buf), "%s: %s", tests[i], msgs[i]);
		else
			snprintf(buf[i], sizeof(*buf), "%s", tests[i]);
		labels[i] = buf[i];
	}
	return (labels);
}

static void	test_exhaustive(void) {
	const char		*tests[] = {
		"signed d/i sweep",
		"unsigned u sweep",
		"hex x/X sweep",
		"string s sweep"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	void			(*fns[4])(void) = {
		exhaustive_signed_test,
		exhaustive_unsigned_test,
		exhaustive_hex_test,
		exhaustive_str_test
	};
	char			msgs[4][256];
	int				passed[4];

	for (size_t i = 0; i < num_tests; i++)
		passed[i] = run_sweep(fns[i], msgs[i]);
	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("exhaustive sweep (bonus)", num_tests,
			build_labels(tests, msgs, num_tests), passed);
}

/* ************************************************************************** */
/*                          extra isolated edge cases                         */
/* ************************************************************************** */

static void	hash_precision_test(void) {
	if (!COMPARE("[%#.5x]", 255)
		|| !COMPARE("[%#.0x]", 0)
		|| !COMPARE("[%#.5X]", 255))
		abort();
}

static void	huge_precision_test(void) {
	if (!COMPARE("[%.40d]", 7)
		|| !COMPARE("[%.40u]", 7))
		abort();
}

static void	pointer_extreme_test(void) {
	if (!COMPARE("[%p]", (void *)-1)
		|| !COMPARE("[%p]", (void *)0x1)
		|| !COMPARE("[%-20p]", (void *)0xdeadbeef))
		abort();
}

static void	test_extra_edges(void) {
	const char		*tests[] = {
		"# with precision (x/X)",
		"huge precision (40)",
		"pointer extremes"
	};
	const size_t	num_tests = ARRAY_SIZE(tests);
	const int		passed[] = {
		!forked_test(hash_precision_test),
		!forked_test(huge_precision_test),
		!forked_test(pointer_extreme_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("extra edges (bonus)", num_tests, tests, passed);
}

int	main(void) {
	test_flag_minus();
	test_flag_zero();
	test_flag_hash();
	test_flag_space();
	test_flag_plus();
	test_width();
	test_precision();
	test_combinations();
	test_misc_flags();
	test_extra_edges();
	test_exhaustive();
	return (g_tests_failed ? 1 : 0);
}
