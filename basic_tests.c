#include "printf_fairy.h"
#include <limits.h>
#include <stdlib.h>
#include <fcntl.h>

#ifndef VERBOSE
# define VERBOSE 0
#endif

/* ************************************************************************** */
/*                              ft_printf %c                                  */
/* ************************************************************************** */

static void	c_basic_test(void) {
	if (!compare("[%c]", 'A'))
		abort();
}

static void	c_null_byte_test(void) {
	if (!compare("[%c]", '\0'))
		abort();
}

static void	c_low_ascii_test(void) {
	if (!compare("[%c]", 1) || !compare("[%c]", 31))
		abort();
}

static void	c_high_ascii_test(void) {
	if (!compare("[%c]", 127))
		abort();
}

static void	test_c(void) {
	const char		*tests[] = {
		"basic",
		"null byte",
		"low ASCII",
		"high ASCII"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
	const int		passed[] = {
		!forked_test(c_basic_test),
		!forked_test(c_null_byte_test),
		!forked_test(c_low_ascii_test),
		!forked_test(c_high_ascii_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("ft_printf %c", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                              ft_printf %s                                  */
/* ************************************************************************** */

static void	s_basic_test(void) {
	if (!compare("[%s]", "hello"))
		abort();
}

static void	s_empty_test(void) {
	if (!compare("[%s]", ""))
		abort();
}

static void	s_null_test(void) {
	if (!compare("[%s]", (char *)NULL))
		abort();
}

static void	s_long_test(void) {
	if (!compare("[%s]", "abcdefghijklmnopqrstuvwxyz0123456789"))
		abort();
}

static void	test_s(void) {
	const char		*tests[] = {
		"basic",
		"empty",
		"NULL",
		"long string"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
	const int		passed[] = {
		!forked_test(s_basic_test),
		!forked_test(s_empty_test),
		!forked_test(s_null_test),
		!forked_test(s_long_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("ft_printf %s", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                              ft_printf %p                                  */
/* ************************************************************************** */

static void	p_null_test(void) {
	if (!compare("[%p]", (void *)NULL))
		abort();
}

static void	p_small_test(void) {
	if (!compare("[%p]", (void *)0x42))
		abort();
}

static void	p_large_test(void) {
	if (!compare("[%p]", (void *)0xdeadbeef))
		abort();
}

static void	p_stack_test(void) {
	int	x = 0;
	if (!compare("[%p]", (void *)&x))
		abort();
}

static void	test_p(void) {
	const char		*tests[] = {
		"NULL",
		"small address",
		"large address",
		"stack address"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
	const int		passed[] = {
		!forked_test(p_null_test),
		!forked_test(p_small_test),
		!forked_test(p_large_test),
		!forked_test(p_stack_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("ft_printf %p", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                            ft_printf %d / %i                               */
/* ************************************************************************** */

static void	d_zero_test(void) {
	if (!compare("[%d]", 0))
		abort();
}

static void	d_positive_test(void) {
	if (!compare("[%d]", 42))
		abort();
}

static void	d_negative_test(void) {
	if (!compare("[%d]", -42))
		abort();
}

static void	d_int_max_test(void) {
	if (!compare("[%d]", INT_MAX))
		abort();
}

static void	d_int_min_test(void) {
	if (!compare("[%d]", INT_MIN))
		abort();
}

static void	i_basic_test(void) {
	if (!compare("[%i]", -42) || !compare("[%i]", 0))
		abort();
}

static void	test_d_i(void) {
	const char		*tests[] = {
		"d zero",
		"d positive",
		"d negative",
		"d INT_MAX",
		"d INT_MIN",
		"i basic"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
	const int		passed[] = {
		!forked_test(d_zero_test),
		!forked_test(d_positive_test),
		!forked_test(d_negative_test),
		!forked_test(d_int_max_test),
		!forked_test(d_int_min_test),
		!forked_test(i_basic_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("ft_printf %d / %i", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                              ft_printf %u                                  */
/* ************************************************************************** */

static void	u_zero_test(void) {
	if (!compare("[%u]", 0u))
		abort();
}

static void	u_positive_test(void) {
	if (!compare("[%u]", 42u))
		abort();
}

static void	u_uint_max_test(void) {
	if (!compare("[%u]", UINT_MAX))
		abort();
}

static void	test_u(void) {
	const char		*tests[] = {
		"zero",
		"positive",
		"UINT_MAX"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
	const int		passed[] = {
		!forked_test(u_zero_test),
		!forked_test(u_positive_test),
		!forked_test(u_uint_max_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("ft_printf %u", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                            ft_printf %x / %X                               */
/* ************************************************************************** */

static void	x_zero_test(void) {
	if (!compare("[%x]", 0) || !compare("[%X]", 0))
		abort();
}

static void	x_basic_test(void) {
	if (!compare("[%x]", 255) || !compare("[%X]", 255))
		abort();
}

static void	x_uint_max_test(void) {
	if (!compare("[%x]", UINT_MAX) || !compare("[%X]", UINT_MAX))
		abort();
}

static void	x_mixed_case_test(void) {
	if (!compare("%x %X", 0xabcdef, 0xabcdef))
		abort();
}

static void	test_x_X(void) {
	const char		*tests[] = {
		"zero",
		"basic 255",
		"UINT_MAX",
		"mixed lower/upper"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
	const int		passed[] = {
		!forked_test(x_zero_test),
		!forked_test(x_basic_test),
		!forked_test(x_uint_max_test),
		!forked_test(x_mixed_case_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("ft_printf %x / %X", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                              ft_printf %%                                  */
/* ************************************************************************** */

static void	percent_basic_test(void) {
	if (!compare("[%%]"))
		abort();
}

static void	percent_mixed_test(void) {
	if (!compare("100%% done"))
		abort();
}

static void	percent_consecutive_test(void) {
	if (!compare("%%%%"))
		abort();
}

static void	test_percent(void) {
	const char		*tests[] = {
		"basic",
		"with text",
		"consecutive"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
	const int		passed[] = {
		!forked_test(percent_basic_test),
		!forked_test(percent_mixed_test),
		!forked_test(percent_consecutive_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("ft_printf %%", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                                Mix tests                                   */
/* ************************************************************************** */

static void	mix_simple_test(void) {
	if (!compare("%s = %d", "answer", 42))
		abort();
}

static void	mix_all_types_test(void) {
	if (!compare("c=%c s=%s d=%d x=%x p=%p",
			'X', "hi", -1, 0xff, (void *)0x42))
		abort();
}

static void	mix_no_format_test(void) {
	if (!compare("just plain text"))
		abort();
}

static void	mix_only_newlines_test(void) {
	if (!compare("\n\n\n"))
		abort();
}

static void	test_mix(void) {
	const char		*tests[] = {
		"simple",
		"all types",
		"no format",
		"only newlines"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
	const int		passed[] = {
		!forked_test(mix_simple_test),
		!forked_test(mix_all_types_test),
		!forked_test(mix_no_format_test),
		!forked_test(mix_only_newlines_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("ft_printf mix", num_tests, tests, passed);
}

/* ************************************************************************** */
/*                          ft_printf return value                            */
/* ************************************************************************** */

/*
 * Sur un fd casse (ici fd 1 ferme), le vrai printf detecte l'echec de write
 * et renvoie -1, meme quand la sortie est vide (write de 0 octet sur un fd
 * invalide renvoie -1). Ces tests s'executent dans un process forke : on
 * ferme stdout, donc on ne peut rien afficher dans l'enfant — abort() en cas
 * d'echec suffit, le parent (stdout intact) affiche le bilan.
 */

static void	ret_broken_fd_empty_test(void) {
	close(1);
	if (ft_printf("") != -1)
		abort();
}

static void	ret_broken_fd_empty_str_test(void) {
	close(1);
	if (ft_printf("%s", "") != -1)
		abort();
}

static void	ret_broken_fd_text_test(void) {
	close(1);
	if (ft_printf("hello world") != -1)
		abort();
}

static void	ret_normal_test(void) {
	int	saved;
	int	devnull;
	int	ret;

	saved = dup(1);
	devnull = open("/dev/null", 1);
	dup2(devnull, 1);
	close(devnull);
	ret = ft_printf("abc%d", 42);
	dup2(saved, 1);
	close(saved);
	if (ret != 5)
		abort();
}

static void	test_return_value(void) {
	const char		*tests[] = {
		"empty output on broken fd",
		"empty %s on broken fd",
		"text on broken fd",
		"normal output count"
	};
	const size_t	num_tests = sizeof(tests) / sizeof(*tests);
	const int		passed[] = {
		!forked_test(ret_broken_fd_empty_test),
		!forked_test(ret_broken_fd_empty_str_test),
		!forked_test(ret_broken_fd_text_test),
		!forked_test(ret_normal_test)
	};

	if (!all_tests_passed(passed, num_tests) || VERBOSE)
		print_test_results("ft_printf return value", num_tests, tests, passed);
}

int	main(void) {
	test_c();
	test_s();
	test_p();
	test_d_i();
	test_u();
	test_x_X();
	test_percent();
	test_mix();
	test_return_value();
	return (g_tests_failed ? 1 : 0);
}
