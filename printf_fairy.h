#ifndef PRINTF_FAIRY_H
# define PRINTF_FAIRY_H

// =============================
// 📚 Libraries
// =============================
# include "ft_printf.h"
# include <stdarg.h>
# include <stddef.h>

// =============================
// 🎨 Global defines
// =============================
# define GREEN			"\033[0;32m"
# define RED			"\033[0;31m"
# define RESET			"\033[0m"

// =============================
// 🌍 Global variables
// =============================
extern int	g_malloc_wrap_enabled;
extern int	g_malloc_count;
extern int	g_malloc_fail_at;
extern int	g_tests_failed;

// =============================
// 📋 Function prototypes
// =============================
void		*__real_malloc(size_t size);
void		*__wrap_malloc(size_t size);
int			all_tests_passed(const int *passed, const size_t num_tests);
int			compare(const char *fmt, ...);
void		print_test_results(char *function_name, const size_t num_tests,
				const char *tests[], const int passed[]);
void		error_exit(char *msg);
int			forked_test(void (*test_func)(void));

#endif //PRINTF_FAIRY_H
