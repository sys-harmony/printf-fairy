#include "printf_fairy.h"
#include <limits.h>

/* ************************************************************************** */
/*                              flag minus (-)                                */
/* ************************************************************************** */

static void	minus_c_test(void) {
	CHECK("[%-5c]", 'A');
}

static void	minus_s_test(void) {
	CHECK("[%-10s]", "hi");
}

static void	minus_d_test(void) {
	CHECK("[%-10d]", 42);
	CHECK("[%-10d]", -42);
}

static void	minus_x_test(void) {
	CHECK("[%-10x]", 255);
}

static const t_test	g_minus_tests[] = {
	{"with %c", minus_c_test},
	{"with %s", minus_s_test},
	{"with %d", minus_d_test},
	{"with %x", minus_x_test},
};

/* ************************************************************************** */
/*                              flag zero (0)                                 */
/* ************************************************************************** */

static void	zero_d_test(void) {
	CHECK("[%010d]", 42);
}

static void	zero_d_negative_test(void) {
	CHECK("[%010d]", -42);
}

static void	zero_x_test(void) {
	CHECK("[%08x]", 255);
}

static void	zero_minus_test(void) {
	CHECK("[%-010d]", 42);
}

static void	zero_precision_test(void) {
	CHECK("[%010.5d]", 42);
}

static void	zero_precision_zero_value_test(void) {
	CHECK("[%05.0d]", 0);
	CHECK("[%05.2d]", 0);
}

static void	zero_precision_short_test(void) {
	CHECK("[%05.1d]", 42);
	CHECK("[%08.2x]", 255);
}

static const t_test	g_zero_tests[] = {
	{"basic", zero_d_test},
	{"with negative", zero_d_negative_test},
	{"with %x", zero_x_test},
	{"ignored if -", zero_minus_test},
	{"ignored if precision", zero_precision_test},
	{"ignored if precision (zero value)", zero_precision_zero_value_test},
	{"ignored if precision (prec <= digits)", zero_precision_short_test},
};

/* ************************************************************************** */
/*                              flag hash (#)                                 */
/* ************************************************************************** */

static void	hash_x_test(void) {
	CHECK("[%#x]", 255);
	CHECK("[%#X]", 255);
}

static void	hash_zero_test(void) {
	CHECK("[%#x]", 0);
	CHECK("[%#X]", 0);
}

static void	hash_width_test(void) {
	CHECK("[%#10x]", 255);
}

static void	hash_zero_pad_test(void) {
	CHECK("[%#010x]", 255);
	CHECK("[%#010X]", 255);
}

static void	hash_precision_test(void) {
	CHECK("[%#.5x]", 255);
	CHECK("[%#.0x]", 0);
	CHECK("[%#.5X]", 255);
}

static const t_test	g_hash_tests[] = {
	{"basic", hash_x_test},
	{"with zero (no prefix)", hash_zero_test},
	{"with width", hash_width_test},
	{"with zero pad", hash_zero_pad_test},
	{"with precision", hash_precision_test},
};

/* ************************************************************************** */
/*                              flag space ( )                                */
/* ************************************************************************** */

static void	space_positive_test(void) {
	CHECK("[% d]", 42);
}

static void	space_negative_test(void) {
	CHECK("[% d]", -42);
}

static void	space_zero_test(void) {
	CHECK("[% d]", 0);
}

static void	space_overridden_by_plus_test(void) {
	CHECK("[%+ d]", 42);
	CHECK("[% +d]", 42);
}

static const t_test	g_space_tests[] = {
	{"with positive", space_positive_test},
	{"with negative", space_negative_test},
	{"with zero", space_zero_test},
	{"overridden by +", space_overridden_by_plus_test},
};

/* ************************************************************************** */
/*                              flag plus (+)                                 */
/* ************************************************************************** */

static void	plus_positive_test(void) {
	CHECK("[%+d]", 42);
}

static void	plus_negative_test(void) {
	CHECK("[%+d]", -42);
}

static void	plus_zero_test(void) {
	CHECK("[%+d]", 0);
}

static void	plus_zero_pad_test(void) {
	CHECK("[%+010d]", 42);
}

static const t_test	g_plus_tests[] = {
	{"with positive", plus_positive_test},
	{"with negative", plus_negative_test},
	{"with zero", plus_zero_test},
	{"with zero pad", plus_zero_pad_test},
};

/* ************************************************************************** */
/*                                  width                                     */
/* ************************************************************************** */

static void	width_basic_test(void) {
	CHECK("[%10d]", 42);
}

static void	width_too_small_test(void) {
	CHECK("[%2d]", 42424);
}

static void	width_zero_test(void) {
	CHECK("[%0d]", 42);
}

static void	width_with_string_test(void) {
	CHECK("[%10s]", "hi");
}

static void	width_huge_test(void) {
	CHECK("[%1000d]", 42);
	CHECK("[%-1000s]", "x");
}

static const t_test	g_width_tests[] = {
	{"basic", width_basic_test},
	{"too small (ignored)", width_too_small_test},
	{"width 0", width_zero_test},
	{"with string", width_with_string_test},
	{"huge width padding", width_huge_test},
};

/* ************************************************************************** */
/*                                precision                                   */
/* ************************************************************************** */

static void	prec_d_basic_test(void) {
	CHECK("[%.5d]", 42);
}

static void	prec_d_zero_test(void) {
	CHECK("[%.0d]", 0);
	CHECK("[%.0d]", 42);
}

static void	prec_s_truncate_test(void) {
	CHECK("[%.3s]", "hello");
	CHECK("[%.0s]", "hello");
}

static void	prec_s_longer_test(void) {
	CHECK("[%.10s]", "hi");
}

static void	prec_x_test(void) {
	CHECK("[%.5x]", 255);
	CHECK("[%.0x]", 0);
}

static void	prec_no_number_test(void) {
	CHECK("[%.d]", 42);
	CHECK("[%.d]", 0);
}

static void	prec_width_combination_test(void) {
	CHECK("[%10.5d]", 42);
	CHECK("[%10.3s]", "hello");
}

static void	prec_huge_test(void) {
	CHECK("[%.40d]", 7);
	CHECK("[%.40u]", 7);
}

static const t_test	g_precision_tests[] = {
	{"d basic", prec_d_basic_test},
	{"d precision 0", prec_d_zero_test},
	{"s truncation", prec_s_truncate_test},
	{"s precision > len", prec_s_longer_test},
	{"x basic", prec_x_test},
	{"precision no number", prec_no_number_test},
	{"width + precision", prec_width_combination_test},
	{"huge precision (40)", prec_huge_test},
};

/* ************************************************************************** */
/*                            combinations                                    */
/* ************************************************************************** */

static void	combo_all_flags_test(void) {
	CHECK("[%+010.5d]", 42);
}

static void	combo_minus_width_prec_s_test(void) {
	CHECK("[%-10.3s]", "hello");
}

static void	combo_repeated_flags_test(void) {
	CHECK("[%+++d]", 42);
}

static void	combo_mixed_test(void) {
	CHECK("[%5.3s] [%+05d] [%#010x]", "hello", -42, 255);
}

static void	combo_neg_unsigned_test(void) {
	CHECK("[%15.12u]", -1);
	CHECK("[%#x]", -1);
}

static const t_test	g_combo_tests[] = {
	{"all flags + width + precision", combo_all_flags_test},
	{"- width precision with %s", combo_minus_width_prec_s_test},
	{"repeated flags", combo_repeated_flags_test},
	{"mixed format", combo_mixed_test},
	{"negative unsigned + width/precision/hash", combo_neg_unsigned_test},
};

/* ************************************************************************** */
/*                          flags on %p / %s                                  */
/* ************************************************************************** */

// glibc behaviour for flags the standard leaves undefined on these conversions.

static void	pointer_width_test(void) {
	CHECK("[%20p]", (void *)0x42);
	CHECK("[%-20p]", (void *)0x42);
}

static void	pointer_zero_pad_test(void) {
	CHECK("[%010p]", (void *)0x42);
}

static void	string_zero_flag_ignored_test(void) {
	CHECK("[%05s]", "hi");
}

static void	string_null_width_test(void) {
	CHECK("[%10s]", (char *)NULL);
	CHECK("[%-10s]", (char *)NULL);
}

static const t_test	g_misc_tests[] = {
	{"%p with width (- and right)", pointer_width_test},
	{"%p with zero pad", pointer_zero_pad_test},
	{"%s ignores flag 0", string_zero_flag_ignored_test},
	{"%s NULL with width", string_null_width_test},
};

/* ************************************************************************** */
/*                    exhaustive cartesian product (bonus)                    */
/* ************************************************************************** */

/*
 * Inspired by francinette's Fsoares: instead of isolated cases, sweep the full
 * flag x width x precision x value space for each conversion and compare every
 * combination against glibc. The per-conversion flag sets avoid undefined
 * behaviour (+ / space never on unsigned conversions, # only on x/X, no
 * precision on c/p), so a mismatch always means a real bug.
 */

static const char	*g_widths[] = {"", "1", "5", "10", "20"};
static const char	*g_precs[] = {"", ".", ".0", ".1", ".2", ".5", ".10"};
static const char	*g_no_prec[] = {""};

static void	sweep_signed_test(void) {
	const char	*flags[] = {"", "-", "0", "+", " ", "-0", "+0", " 0", "-+"};
	const int	vals[] = {0, 1, -1, 42, -42, INT_MAX, INT_MIN, 7};

	SWEEP("di", flags, g_widths, g_precs, vals);
}

static void	sweep_unsigned_test(void) {
	const char			*flags[] = {"", "-", "0", "-0"};
	const unsigned int	vals[] = {0, 1, 42, 4242, UINT_MAX, 2147483648u};

	SWEEP("u", flags, g_widths, g_precs, vals);
}

static void	sweep_hex_test(void) {
	const char			*flags[] = {"", "-", "0", "#", "-0", "0#", "-#"};
	const unsigned int	vals[] = {0, 1, 255, 0xabcdef, UINT_MAX, 2147483648u};

	SWEEP("xX", flags, g_widths, g_precs, vals);
}

static void	sweep_char_test(void) {
	const char	*flags[] = {"", "-"};
	const int	vals[] = {'a', '\0', 127};

	SWEEP("c", flags, g_widths, g_no_prec, vals);
}

static void	sweep_str_test(void) {
	const char	*flags[] = {"", "-"};
	const char	*precs[] = {"", ".", ".0", ".3", ".10"};
	const char	*vals[] = {"", "a", "hello", "abcdefghijklmnop"};

	SWEEP("s", flags, g_widths, precs, vals);
}

static void	sweep_pointer_test(void) {
	const char	*flags[] = {"", "-"};
	void *const	vals[] = {NULL, (void *)0x42, (void *)-1};

	SWEEP("p", flags, g_widths, g_no_prec, vals);
}

static const t_test	g_sweep_tests[] = {
	{"signed d/i sweep", sweep_signed_test},
	{"unsigned u sweep", sweep_unsigned_test},
	{"hex x/X sweep", sweep_hex_test},
	{"char c sweep", sweep_char_test},
	{"string s sweep", sweep_str_test},
	{"pointer p sweep", sweep_pointer_test},
};

int	main(void) {
	RUN_SECTION("flag - (bonus)", g_minus_tests);
	RUN_SECTION("flag 0 (bonus)", g_zero_tests);
	RUN_SECTION("flag # (bonus)", g_hash_tests);
	RUN_SECTION("flag space (bonus)", g_space_tests);
	RUN_SECTION("flag + (bonus)", g_plus_tests);
	RUN_SECTION("width (bonus)", g_width_tests);
	RUN_SECTION("precision (bonus)", g_precision_tests);
	RUN_SECTION("combinations (bonus)", g_combo_tests);
	RUN_SECTION("misc flags (bonus)", g_misc_tests);
	RUN_SECTION("exhaustive sweep (bonus)", g_sweep_tests);
	return (g_tests_failed != 0);
}
