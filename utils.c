#include "printf_fairy.h"
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>

int			g_malloc_wrap_enabled = 0;
int			g_malloc_count = 0;
int			g_malloc_fail_at = 0;
int			g_tests_failed = 0;

// Shared with forked children so they can explain why they failed.
static char	*g_fail_msg = NULL;
static int	g_saved_stdout;
static int	g_pipefd[2];

void	*__wrap_malloc(size_t size) {
	if (g_malloc_wrap_enabled && ++g_malloc_count == g_malloc_fail_at)
		return NULL;
	return __real_malloc(size);
}

void	error_exit(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void	silence_stdout(void) {
	int	fd = open("/dev/null", O_WRONLY);

	if (fd < 0 || dup2(fd, STDOUT_FILENO) < 0)
		error_exit("printf-fairy: /dev/null");
	close(fd);
}

// Records why the current test failed, then aborts the forked child.
void	test_fail(const char *fmt, ...) {
	va_list	ap;

	if (g_fail_msg) {
		va_start(ap, fmt);
		vsnprintf(g_fail_msg, FAIL_MSG_SIZE, fmt, ap);
		va_end(ap);
	}
	abort();
}

/* ************************************************************************** */
/*                              stdout capture                                */
/* ************************************************************************** */

void	capture_start(void) {
	fflush(stdout);
	g_saved_stdout = dup(STDOUT_FILENO);
	if (g_saved_stdout == -1 || pipe(g_pipefd) == -1)
		error_exit("printf-fairy: capture failed");
	dup2(g_pipefd[1], STDOUT_FILENO);
	close(g_pipefd[1]);
}

void	capture_end(t_capture *cap) {
	ssize_t	n;

	fflush(stdout);
	dup2(g_saved_stdout, STDOUT_FILENO);
	close(g_saved_stdout);
	cap->len = 0;
	while (cap->len < CAPTURE_SIZE
		&& (n = read(g_pipefd[0], cap->buf + cap->len,
				CAPTURE_SIZE - cap->len)) > 0)
		cap->len += n;
	close(g_pipefd[0]);
}

// Writes src as a quoted literal, non-printable bytes as \xHH, max 60 bytes.
static void	quote(char *dst, size_t size, const char *src, ssize_t len) {
	size_t	n = snprintf(dst, size, "\"");

	for (ssize_t i = 0; i < len && i < 60; i++) {
		unsigned char	c = src[i];

		if (c == '\n')
			n += snprintf(dst + n, size - n, "\\n");
		else if (c == '"' || c == '\\')
			n += snprintf(dst + n, size - n, "\\%c", c);
		else if (isprint(c))
			n += snprintf(dst + n, size - n, "%c", c);
		else
			n += snprintf(dst + n, size - n, "\\x%02x", c);
	}
	snprintf(dst + n, size - n, len > 60 ? "\"..." : "\"");
}

// Byte-exact comparison (a %c of '\0' must not end the comparison early).
void	check_output(const char *fmt, const t_capture *ft,
			const t_capture *libc) {
	char	f[256];
	char	exp[256];
	char	got[256];
	ssize_t	i = 0;
	ssize_t	from;

	while (i < ft->len && i < libc->len && ft->buf[i] == libc->buf[i])
		i++;
	if (i == ft->len && i == libc->len && ft->ret == libc->ret)
		return ;
	from = i > 20 ? i - 20 : 0;
	quote(f, sizeof(f), fmt, strlen(fmt));
	quote(exp, sizeof(exp), libc->buf + from, libc->len - from);
	quote(got, sizeof(got), ft->buf + from, ft->len - from);
	test_fail("      format:   %s\n"
		"      expected: %s%s (ret %d)\n"
		"      got:      %s%s (ret %d)",
		f, from ? "..." : "", exp, libc->ret, from ? "..." : "", got, ft->ret);
}

/* ************************************************************************** */
/*                              test runner                                   */
/* ************************************************************************** */

// Runs fn in a child process. Returns 1 if it exited normally, else 0.
static int	run_forked(void (*fn)(void)) {
	pid_t	pid;
	int		status;
	int		sig;

	g_fail_msg[0] = '\0';
	fflush(stdout);
	pid = fork();
	if (pid == -1)
		error_exit("printf-fairy: fork failed");
	if (!pid) {
		alarm(TIMEOUT_SEC);
		fn();
		exit(EXIT_SUCCESS);
	}
	if (waitpid(pid, &status, 0) == -1)
		error_exit("printf-fairy: waitpid failed");
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return 1;
	sig = WIFSIGNALED(status) ? WTERMSIG(status) : 0;
	if (sig == SIGALRM)
		snprintf(g_fail_msg, FAIL_MSG_SIZE,
			"      timeout after %ds (infinite loop?)", TIMEOUT_SEC);
	else if (sig && sig != SIGABRT)
		snprintf(g_fail_msg, FAIL_MSG_SIZE, "      crashed: %s",
			strsignal(sig));
	return 0;
}

// Prints the section only when something failed, or every test in VERBOSE.
void	run_section(const char *title, const t_test *tests, size_t n) {
	int	header = 0;
	int	ok;

	if (!g_fail_msg) {
		g_fail_msg = mmap(NULL, FAIL_MSG_SIZE, PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		if (g_fail_msg == MAP_FAILED)
			error_exit("printf-fairy: mmap failed");
	}
	for (size_t i = 0; i < n; i++) {
		ok = run_forked(tests[i].fn);
		g_tests_failed += !ok;
		if (ok && !VERBOSE)
			continue;
		if (!header++)
			printf("\n========================================\n"
				"%s\n========================================\n", title);
		printf("%s" RESET "  Test %s\n", ok ? GREEN "[OK]" : RED "[KO]",
			tests[i].name);
		if (!ok && g_fail_msg[0])
			printf("%s\n", g_fail_msg);
	}
}
