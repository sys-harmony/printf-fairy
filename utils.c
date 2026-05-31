#include "printf_fairy.h"

int	g_malloc_wrap_enabled = 0;
int	g_malloc_count;
int	g_malloc_fail_at;
int	g_tests_failed = 0;

void	*__wrap_malloc(size_t size) {
	if (g_malloc_wrap_enabled && ++g_malloc_count == g_malloc_fail_at)
		return NULL;
	return __real_malloc(size);
}

int	all_tests_passed(const int *passed, const size_t num_tests) {
	for (size_t i = 0; i < num_tests; i++) {
		if (!passed[i])
			return 0;
	}
	return 1;
}

#include <stdio.h>
#include <string.h>

int	compare(const char *fmt, ...) {
	char	ft_buf[2048] = {0};
	char	libc_buf[2048] = {0};
	va_list	args;
	int		ft_ret;
	int		libc_ret;

	va_start(args, fmt);
	ft_ret = ft_vsnprintf(ft_buf, sizeof(ft_buf), fmt, args);
	va_end(args);
	va_start(args, fmt);
	libc_ret = vsnprintf(libc_buf, sizeof(libc_buf), fmt, args);
	va_end(args);
	return (ft_ret == libc_ret) && (strcmp(ft_buf, libc_buf) == 0);
}

void	print_test_results(char *function_name, const size_t num_tests, const char *tests[], const int passed[]) {
	printf("\n========================================\n");
	printf("%s\n", function_name);
	printf("========================================\n");
	for (size_t i = 0; i < num_tests; i++) {
		printf("%s" RESET "  Test %s\n", passed[i] ? GREEN "[OK]" : RED "[KO]", tests[i]);
		if (!passed[i])
			g_tests_failed++;
	}
}

#include <stdlib.h>

void	error_exit(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

#include <unistd.h>
#include <sys/wait.h>

static pid_t	xwaitpid(pid_t pid, int *status, int options)
{
	pid_t    ret = waitpid(pid, status, options);
	if (ret == -1)
		error_exit("printf-fairy: waitpid failed");
	return ret;
}

int	forked_test(void (*test_func)(void)) {
	pid_t	pid;
	int		status;

	fflush(stdout);
	pid = fork();
	if (pid == -1)
		error_exit("printf-fairy: fork failed");
	if (!pid) {
		test_func();
		exit(EXIT_SUCCESS);
	}
	xwaitpid(pid, &status, 0);
	if (WIFSIGNALED(status))
		return 1;
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		return 1;
	return 0;
}
