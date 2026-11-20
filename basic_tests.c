#include "printf_fairy.h"

/*
 * Mandatory conversions. Each test sweeps a conversion over values picked at
 * the edges (0, ±1, powers of the base, INT_MIN...) and compares ft_printf
 * with printf byte for byte, return value included.
 */

static const char		*g_none[] = {""};
static const int		g_ints[] = {0, 1, -1, 7, 9, 10, -10, 42, -42, 99, 100,
	-100, 12345, -12345, 999999, 1000000, INT_MAX, INT_MIN, INT_MAX - 1, INT_MIN + 1};
static const unsigned	g_uints[] = {0, 1, 7, 9, 10, 15, 16, 42, 255, 256, 4095,
	4096, 65535, 65536, 0xabcdef, 0x7fffffff, 0x80000000, 0xdeadbeef, UINT_MAX - 1, UINT_MAX};

/* ************************************************************************** */
/*                               conversions                                  */
/* ************************************************************************** */

static void	c_test(void) {
	for (int c = 0; c < 256; c++)
		CHECK("[%c]", c);
}

static void	c_nul_test(void) {
	CHECK("%c", '\0');
	CHECK("%c%c%c", 'a', '\0', 'b');
	CHECK("%c%c", '\0', '\0');
}

static void	s_test(void) {
	const char	*vals[] = {"", "a", "hello", " \t\n", "\x01\x7f\x80\xff", "%d %s %%"};

	SWEEP("s", g_none, g_none, g_none, vals);
	CHECK("[%s]", (char *)NULL);
}

static void	s_long_test(void) {
	char	big[3000];

	memset(big, 'x', sizeof(big) - 1);
	big[sizeof(big) - 1] = '\0';
	CHECK("%s", big);
	CHECK("[%s|%s]", big, big);
}

/* Top bit set: catches pointers printed through a signed long ("0x-1"). */
static void	p_test(void) {
	int			stack;
	void		*heap = malloc(1);
	void *const	vals[] = {NULL, (void *)1, (void *)0x42, (void *)0xdeadbeef,
		(void *)-1, (void *)LONG_MIN, (void *)LONG_MAX, &stack, heap};

	SWEEP("p", g_none, g_none, g_none, vals);
	free(heap);
}

static void	di_test(void) {
	SWEEP("di", g_none, g_none, g_none, g_ints);
}

static void	u_test(void) {
	SWEEP("u", g_none, g_none, g_none, g_uints);
	CHECK("[%u]", -1);
	CHECK("[%u]", INT_MIN);
}

static void	x_test(void) {
	SWEEP("xX", g_none, g_none, g_none, g_uints);
	CHECK("[%x %X]", -1, INT_MIN);
}

static void	percent_test(void) {
	CHECK("%%");
	CHECK("100%%");
	CHECK("%%%%");
	CHECK("%%d %%s");
	CHECK("[%%%c%%]", 'x');
}

static const t_test	g_conversion_tests[] = {
	{"%c, every byte from 0 to 255", c_test},
	{"%c of '\\0' among other characters", c_nul_test},
	{"%s: empty, spaces, high bytes, a format, NULL", s_test},
	{"%s: 3000 characters", s_long_test},
	{"%p: NULL, top bit set, stack, heap", p_test},
	{"%d %i: 0, ±1, powers of 10, INT_MIN, INT_MAX", di_test},
	{"%u: powers of 2 and 16, UINT_MAX, negative arguments", u_test},
	{"%x %X: powers of 2 and 16, UINT_MAX, negative arguments", x_test},
	{"%%: alone, doubled, around text", percent_test},
};

/* ************************************************************************** */
/*                                 formats                                    */
/* ************************************************************************** */

static void	text_test(void) {
	CHECK("");
	CHECK("just plain text");
	CHECK("\n\n\n");
}

static void	long_text_test(void) {
	char	fmt[5000];

	memset(fmt, 'z', sizeof(fmt) - 1);
	fmt[sizeof(fmt) - 1] = '\0';
	CHECK(fmt);
}

static void	all_conversions_test(void) {
	CHECK("c=%c s=%s p=%p d=%d i=%i u=%u x=%x X=%X %%", 'Z', "str", (void *)0x42,
		-42, INT_MIN, UINT_MAX, 0xabc, 0xabc);
	CHECK("%s%d%s%x%c", "a", 1, "b", 255, '!');
	CHECK("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
}

static const t_test	g_format_tests[] = {
	{"empty format, text only, newlines", text_test},
	{"5000 characters of text", long_text_test},
	{"every conversion in a format, 20 in a row", all_conversions_test},
};

/* ************************************************************************** */
/*                               robustness                                   */
/* ************************************************************************** */

/*
 * With stdout closed, write() fails and an unbuffered printf returns -1. The
 * subject does not mention write errors: a difference only warns.
 */
static void	closed_stdout_test(void) {
	int	ret;

	close(STDOUT_FILENO);
	ret = ft_printf("hello world");
	if (ret != -1)
		warning("returned %d with stdout closed, where printf returns -1", ret);
}

/*
 * Each malloc failing in turn: ft_printf may give up, but not crash (leaks
 * are Valgrind's, in leak_tests). One that never calls malloc passes as is.
 */
static void	malloc_fail_test(void) {
	char	big[10000];

	memset(big, 'a', sizeof(big) - 1);
	big[sizeof(big) - 1] = '\0';
	silence_stdout();
	EACH_MALLOC_FAILING(ft_printf("%s %d %s %p %c %u %x %X %%\n", big, 42, "x", (void *)big, 'c', 7u, 255u, 255u));
	EACH_MALLOC_FAILING(ft_printf("short %d\n", 42));
}

static const t_test	g_robustness_tests[] = {
	{"stdout closed: returns -1", closed_stdout_test},
	{"malloc failing: no crash", malloc_fail_test},
};

/* ************************************************************************** */
/*                           outside the subject                              */
/* ************************************************************************** */

/*
 * Undefined in C, yet often tried in evaluation: a crash, a hang or a return
 * value other than glibc's only warns (the group runs lenient, last).
 */

/* fmt copied to the very end of a page, before an unreadable one: reading
 * past its '\0' crashes instead of going unnoticed. */
static const char	*at_page_end(const char *fmt) {
	long	page = sysconf(_SC_PAGESIZE);
	size_t	len = strlen(fmt) + 1;
	char	*p = mmap(NULL, 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (p == MAP_FAILED || mprotect(p + page, page, PROT_NONE) < 0)
		die("printf-fairy: mmap");
	return memcpy(p + page - len, fmt, len);
}

static void	null_format_test(void) {
	int	ret;

	silence_stdout();
	ret = ft_printf(NULL);
	if (ret != -1)
		warning("returned %d, where printf returns -1", ret);
}

static void	lone_percent_test(void) {
	silence_stdout();
	ft_printf(at_page_end("abc%"));
}

static void	unknown_conversion_test(void) {
	silence_stdout();
	ft_printf(at_page_end("[%k]"));
}

static const t_test	g_outside_tests[] = {
	{"ft_printf(NULL): -1 like printf", null_format_test},
	{"\"abc%\": nothing read past the format", lone_percent_test},
	{"\"[%k]\": unknown conversion, no crash", unknown_conversion_test},
};

int	main(void) {
	RUN_GROUP("ft_printf conversions", g_conversion_tests);
	RUN_GROUP("ft_printf formats", g_format_tests);
	RUN_GROUP("ft_printf robustness", g_robustness_tests);
	g_lenient = 1;
	RUN_GROUP("ft_printf outside the subject (warnings)", g_outside_tests);
	return fairy_summary();
}
