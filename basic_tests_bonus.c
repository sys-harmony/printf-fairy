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
	if (!compare("[%-5c]", 'A'))
		abort();
}

static void	minus_s_test(void) {
	if (!compare("[%-10s]", "hi"))
		abort();
}

static void	minus_d_test(void) {
	if (!compare("[%-10d]", 42) || !compare("[%-10d]", -42))
		abort();
}

static void	minus_x_test(void) {
	if (!compare("[%-10x]", 255))
		abort();
}

static void	test_flag_minus(void) {
	const char		*tests[] = {
		"with %c",
		"with %s",
		"with %d",
		"with %x"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
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
	if (!compare("[%010d]", 42))
		abort();
}

static void	zero_d_negative_test(void) {
	if (!compare("[%010d]", -42))
		abort();
}

static void	zero_x_test(void) {
	if (!compare("[%08x]", 255))
		abort();
}

static void	zero_minus_test(void) {
	if (!compare("[%-010d]", 42))
		abort();
}

static void	zero_precision_test(void) {
	if (!compare("[%010.5d]", 42))
		abort();
}

static void	zero_precision_zero_value_test(void) {
	if (!compare("[%05.0d]", 0) || !compare("[%05.2d]", 0))
		abort();
}

static void	test_flag_zero(void) {
	const char		*tests[] = {
		"basic",
		"with negative",
		"with %x",
		"ignored if -",
		"ignored if precision",
		"ignored if precision (zero value)"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
	const int		passed[] = {
		!forked_test(zero_d_test),
		!forked_test(zero_d_negative_test),
		!forked_test(zero_x_test),
		!forked_test(zero_minus_test),
		!forked_test(zero_precision_test),
		!forked_test(zero_precision_zero_value_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("flag 0 (bonus)", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                              flag hash (#)                                 */
/* ************************************************************************** */

static void	hash_x_test(void) {
	if (!compare("[%#x]", 255) || !compare("[%#X]", 255))
		abort();
}

static void	hash_zero_test(void) {
	if (!compare("[%#x]", 0) || !compare("[%#X]", 0))
		abort();
}

static void	hash_width_test(void) {
	if (!compare("[%#10x]", 255))
		abort();
}

static void	hash_zero_pad_test(void) {
	if (!compare("[%#010x]", 255))
		abort();
}

static void	test_flag_hash(void) {
	const char		*tests[] = {
		"basic",
		"with zero (no prefix)",
		"with width",
		"with zero pad"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
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
	if (!compare("[% d]", 42))
		abort();
}

static void	space_negative_test(void) {
	if (!compare("[% d]", -42))
		abort();
}

static void	space_zero_test(void) {
	if (!compare("[% d]", 0))
		abort();
}

static void	space_overridden_by_plus_test(void) {
	if (!compare("[%+ d]", 42) || !compare("[% +d]", 42))
		abort();
}

static void	test_flag_space(void) {
	const char		*tests[] = {
		"with positive",
		"with negative",
		"with zero",
		"overridden by +"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
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
	if (!compare("[%+d]", 42))
		abort();
}

static void	plus_negative_test(void) {
	if (!compare("[%+d]", -42))
		abort();
}

static void	plus_zero_test(void) {
	if (!compare("[%+d]", 0))
		abort();
}

static void	plus_zero_pad_test(void) {
	if (!compare("[%+010d]", 42))
		abort();
}

static void	test_flag_plus(void) {
	const char		*tests[] = {
		"with positive",
		"with negative",
		"with zero",
		"with zero pad"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
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
	if (!compare("[%10d]", 42))
		abort();
}

static void	width_too_small_test(void) {
	if (!compare("[%2d]", 42424))
		abort();
}

static void	width_zero_test(void) {
	if (!compare("[%0d]", 42))
		abort();
}

static void	width_with_string_test(void) {
	if (!compare("[%10s]", "hi"))
		abort();
}

static void	test_width(void) {
	const char		*tests[] = {
		"basic",
		"too small (ignored)",
		"width 0",
		"with string"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
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
	if (!compare("[%.5d]", 42))
		abort();
}

static void	prec_d_zero_test(void) {
	if (!compare("[%.0d]", 0) || !compare("[%.0d]", 42))
		abort();
}

static void	prec_s_truncate_test(void) {
	if (!compare("[%.3s]", "hello") || !compare("[%.0s]", "hello"))
		abort();
}

static void	prec_s_longer_test(void) {
	if (!compare("[%.10s]", "hi"))
		abort();
}

static void	prec_x_test(void) {
	if (!compare("[%.5x]", 255) || !compare("[%.0x]", 0))
		abort();
}

static void	prec_no_number_test(void) {
	if (!compare("[%.d]", 42) || !compare("[%.d]", 0))
		abort();
}

static void	prec_width_combination_test(void) {
	if (!compare("[%10.5d]", 42) || !compare("[%10.3s]", "hello"))
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
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
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
	if (!compare("[%+010.5d]", 42))
		abort();
}

static void	combo_hash_zero_pad_x_test(void) {
	if (!compare("[%#010x]", 255) || !compare("[%#010X]", 255))
		abort();
}

static void	combo_minus_width_prec_s_test(void) {
	if (!compare("[%-10.3s]", "hello"))
		abort();
}

static void	combo_repeated_flags_test(void) {
	if (!compare("[%+++d]", 42))
		abort();
}

static void	combo_mixed_test(void) {
	if (!compare("[%5.3s] [%+05d] [%#010x]", "hello", -42, 255))
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
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
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

int	main(void) {
	test_flag_minus();
	test_flag_zero();
	test_flag_hash();
	test_flag_space();
	test_flag_plus();
	test_width();
	test_precision();
	test_combinations();
	return (g_tests_failed ? 1 : 0);
}
