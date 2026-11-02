#include "printf_fairy.h"
#include <limits.h>

/* ************************************************************************** */
/*                              ft_printf %c                                  */
/* ************************************************************************** */

static void	c_basic_test(void) {
	CHECK("[%c]", 'A');
}

static void	c_null_byte_test(void) {
	CHECK("[%c]", '\0');
	CHECK("%c%c%c", 'a', '\0', 'b');
}

static void	c_low_ascii_test(void) {
	CHECK("[%c]", 1);
	CHECK("[%c]", 31);
}

static void	c_high_ascii_test(void) {
	CHECK("[%c]", 127);
}

static const t_test	g_c_tests[] = {
	{"basic", c_basic_test},
	{"null byte", c_null_byte_test},
	{"low ASCII", c_low_ascii_test},
	{"high ASCII", c_high_ascii_test},
};

/* ************************************************************************** */
/*                              ft_printf %s                                  */
/* ************************************************************************** */

static void	s_basic_test(void) {
	CHECK("[%s]", "hello");
}

static void	s_empty_test(void) {
	CHECK("[%s]", "");
}

static void	s_null_test(void) {
	CHECK("[%s]", (char *)NULL);
}

static void	s_long_test(void) {
	CHECK("[%s]", "abcdefghijklmnopqrstuvwxyz0123456789");
}

static const t_test	g_s_tests[] = {
	{"basic", s_basic_test},
	{"empty", s_empty_test},
	{"NULL", s_null_test},
	{"long string", s_long_test},
};

/* ************************************************************************** */
/*                              ft_printf %p                                  */
/* ************************************************************************** */

static void	p_null_test(void) {
	CHECK("[%p]", (void *)NULL);
}

static void	p_small_test(void) {
	CHECK("[%p]", (void *)0x42);
}

static void	p_large_test(void) {
	CHECK("[%p]", (void *)0xdeadbeef);
}

static void	p_stack_test(void) {
	int	x = 0;

	CHECK("[%p]", (void *)&x);
}

// Top bit set: catches pointers printed through a signed long ("0x-1").
static void	p_extremes_test(void) {
	CHECK("[%p]", (void *)-1);
	CHECK("[%p]", (void *)LONG_MIN);
	CHECK("[%p]", (void *)LONG_MAX);
}

static const t_test	g_p_tests[] = {
	{"NULL", p_null_test},
	{"small address", p_small_test},
	{"large address", p_large_test},
	{"stack address", p_stack_test},
	{"extremes (-1, LONG_MIN, LONG_MAX)", p_extremes_test},
};

/* ************************************************************************** */
/*                            ft_printf %d / %i                               */
/* ************************************************************************** */

static void	d_zero_test(void) {
	CHECK("[%d]", 0);
}

static void	d_positive_test(void) {
	CHECK("[%d]", 42);
}

static void	d_negative_test(void) {
	CHECK("[%d]", -42);
}

static void	d_int_max_test(void) {
	CHECK("[%d]", INT_MAX);
}

static void	d_int_min_test(void) {
	CHECK("[%d]", INT_MIN);
}

static void	i_basic_test(void) {
	CHECK("[%i]", -42);
	CHECK("[%i]", 0);
}

static const t_test	g_d_i_tests[] = {
	{"d zero", d_zero_test},
	{"d positive", d_positive_test},
	{"d negative", d_negative_test},
	{"d INT_MAX", d_int_max_test},
	{"d INT_MIN", d_int_min_test},
	{"i basic", i_basic_test},
};

/* ************************************************************************** */
/*                              ft_printf %u                                  */
/* ************************************************************************** */

static void	u_zero_test(void) {
	CHECK("[%u]", 0u);
}

static void	u_positive_test(void) {
	CHECK("[%u]", 42u);
}

static void	u_uint_max_test(void) {
	CHECK("[%u]", UINT_MAX);
}

static void	u_negative_arg_test(void) {
	CHECK("[%u]", -1);
	CHECK("[%u]", INT_MIN);
}

static const t_test	g_u_tests[] = {
	{"zero", u_zero_test},
	{"positive", u_positive_test},
	{"UINT_MAX", u_uint_max_test},
	{"negative arg", u_negative_arg_test},
};

/* ************************************************************************** */
/*                            ft_printf %x / %X                               */
/* ************************************************************************** */

static void	x_zero_test(void) {
	CHECK("[%x]", 0);
	CHECK("[%X]", 0);
}

static void	x_basic_test(void) {
	CHECK("[%x]", 255);
	CHECK("[%X]", 255);
}

static void	x_uint_max_test(void) {
	CHECK("[%x]", UINT_MAX);
	CHECK("[%X]", UINT_MAX);
}

static void	x_negative_arg_test(void) {
	CHECK("[%x]", -1);
	CHECK("[%X]", INT_MIN);
}

static void	x_mixed_case_test(void) {
	CHECK("%x %X", 0xabcdef, 0xabcdef);
}

static const t_test	g_x_tests[] = {
	{"zero", x_zero_test},
	{"basic 255", x_basic_test},
	{"UINT_MAX", x_uint_max_test},
	{"negative arg", x_negative_arg_test},
	{"mixed lower/upper", x_mixed_case_test},
};

/* ************************************************************************** */
/*                              ft_printf %%                                  */
/* ************************************************************************** */

static void	percent_basic_test(void) {
	CHECK("[%%]");
}

static void	percent_mixed_test(void) {
	CHECK("100%% done");
}

static void	percent_consecutive_test(void) {
	CHECK("%%%%");
}

static const t_test	g_percent_tests[] = {
	{"basic", percent_basic_test},
	{"with text", percent_mixed_test},
	{"consecutive", percent_consecutive_test},
};

/* ************************************************************************** */
/*                                Mix tests                                   */
/* ************************************************************************** */

static void	mix_simple_test(void) {
	CHECK("%s = %d", "answer", 42);
}

static void	mix_all_types_test(void) {
	CHECK("c=%c s=%s d=%d x=%x p=%p", 'X', "hi", -1, 0xff, (void *)0x42);
}

static void	mix_no_format_test(void) {
	CHECK("just plain text");
}

static void	mix_only_newlines_test(void) {
	CHECK("\n\n\n");
}

static const t_test	g_mix_tests[] = {
	{"simple", mix_simple_test},
	{"all types", mix_all_types_test},
	{"no format", mix_no_format_test},
	{"only newlines", mix_only_newlines_test},
};

/* ************************************************************************** */
/*                              robustness                                    */
/* ************************************************************************** */

/*
 * With stdout closed, an unbuffered printf("hello world") returns -1: since
 * ft_printf writes directly (no buffer management), it must do the same.
 * Empty output is not tested: glibc returns 0 there, as nothing is written.
 */
static void	ret_broken_fd_test(void) {
	int	ret;

	close(STDOUT_FILENO);
	ret = ft_printf("hello world");
	if (ret != -1)
		test_fail("      returned %d with stdout closed, expected -1", ret);
}

/*
 * Makes the 1st, then 2nd, ... malloc fail: ft_printf may give up, but must
 * not crash. Implementations that never call malloc pass trivially.
 */
static void	malloc_fail_test(void) {
	char	big[10000];

	memset(big, 'a', sizeof(big) - 1);
	big[sizeof(big) - 1] = '\0';
	silence_stdout();
	g_malloc_wrap_enabled = 1;
	for (g_malloc_fail_at = 1; g_malloc_fail_at <= 5; g_malloc_fail_at++) {
		g_malloc_count = 0;
		ft_printf("%s %d %s %p\n", big, 42, "x", (void *)big);
	}
}

static const t_test	g_robustness_tests[] = {
	{"-1 when stdout is closed", ret_broken_fd_test},
	{"no crash when malloc fails", malloc_fail_test},
};

int	main(void) {
	RUN_SECTION("ft_printf %c", g_c_tests);
	RUN_SECTION("ft_printf %s", g_s_tests);
	RUN_SECTION("ft_printf %p", g_p_tests);
	RUN_SECTION("ft_printf %d / %i", g_d_i_tests);
	RUN_SECTION("ft_printf %u", g_u_tests);
	RUN_SECTION("ft_printf %x / %X", g_x_tests);
	RUN_SECTION("ft_printf %%", g_percent_tests);
	RUN_SECTION("ft_printf mix", g_mix_tests);
	RUN_SECTION("ft_printf robustness", g_robustness_tests);
	return (g_tests_failed != 0);
}
