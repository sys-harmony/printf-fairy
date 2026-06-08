#ifndef PRINTF_FAIRY_H
# define PRINTF_FAIRY_H

// =============================
// 📚 Libraries
// =============================
# include "ft_printf.h"
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <stdarg.h>
# include <stddef.h>

// =============================
// 🎨 Global define(s)
// =============================
# define GREEN			"\033[0;32m"
# define RED			"\033[0;31m"
# define RESET			"\033[0m"

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
# define COMPARE(...) \
	({ \
		int ft_ret, libc_ret; \
		char ft_buf[2048] = {0}; \
		char libc_buf[2048] = {0}; \
		capture_start(); \
		ft_ret = ft_printf(__VA_ARGS__); \
		capture_end(ft_buf); \
		capture_start(); \
		libc_ret = printf(__VA_ARGS__); \
		capture_end(libc_buf); \
		((ft_ret == libc_ret) && (strcmp(ft_buf, libc_buf) == 0)); \
	})

// =============================
// 📋 Function prototype(s)
// =============================
void		capture_start(void);
void		capture_end(char *buf);
void		*__real_malloc(size_t size);
void		*__wrap_malloc(size_t size);
int			all_tests_passed(const int *passed, const size_t num_tests);
void		print_test_results(char *function_name, const size_t num_tests,
				const char *tests[], const int passed[]);
void		error_exit(char *msg);
int			forked_test(void (*test_func)(void));

#endif //PRINTF_FAIRY_H
