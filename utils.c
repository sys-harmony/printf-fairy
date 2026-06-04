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

static int	g_saved_stdout;
static int	g_pipefd[2];

void	capture_start(void)
{
	fflush(stdout);
	g_saved_stdout = dup(STDOUT_FILENO);
	if (pipe(g_pipefd) == -1)
		error_exit("printf-fairy: pipe failed");
	dup2(g_pipefd[1], STDOUT_FILENO);
	close(g_pipefd[1]);
}

void	capture_end(char *buf)
{
	int	bytes;

	fflush(stdout);
	dup2(g_saved_stdout, STDOUT_FILENO);
	close(g_saved_stdout);
	
	bytes = read(g_pipefd[0], buf, 2047);
	if (bytes >= 0)
		buf[bytes] = '\0';
	close(g_pipefd[0]);
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
