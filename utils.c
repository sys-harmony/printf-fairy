#include "printf_fairy.h"
#include <fcntl.h>

int	g_malloc_wrap_enabled = 0;
int	g_malloc_count = 0;
int	g_malloc_fail_at = 0;

static int	g_saved_stdout;
static int	g_pipefd[2];

/* With -Wl,--wrap=malloc, the n-th malloc (g_malloc_fail_at) returns NULL. */
void	*__wrap_malloc(size_t size) {
	if (g_malloc_wrap_enabled && ++g_malloc_count == g_malloc_fail_at)
		return NULL;
	return __real_malloc(size);
}

void	silence_stdout(void) {
	int	fd = open("/dev/null", O_WRONLY);

	if (fd < 0 || dup2(fd, STDOUT_FILENO) < 0)
		die("printf-fairy: /dev/null");
	close(fd);
}

/* ************************************************************************** */
/*                              stdout capture                                */
/* ************************************************************************** */

void	capture_start(void) {
	fflush(stdout);
	g_saved_stdout = dup(STDOUT_FILENO);
	if (g_saved_stdout < 0 || pipe(g_pipefd) < 0)
		die("printf-fairy: capture");
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
		&& (n = read(g_pipefd[0], cap->buf + cap->len, CAPTURE_SIZE - cap->len)) > 0)
		cap->len += n;
	close(g_pipefd[0]);
}

/* ************************************************************************** */
/*                                comparison                                  */
/* ************************************************************************** */

/* Quotes len bytes of src (at most 60), non-printable ones as \xHH. */
static const char	*quote(char *dst, const char *src, ssize_t len, int cut) {
	size_t	n = sprintf(dst, "%s\"", cut ? "…" : "");

	for (ssize_t i = 0; i < len && i < 60; i++) {
		unsigned char	c = src[i];

		if (c == '\n')
			n += sprintf(dst + n, "\\n");
		else if (c == '"' || c == '\\')
			n += sprintf(dst + n, "\\%c", c);
		else if (isprint(c))
			n += sprintf(dst + n, "%c", c);
		else
			n += sprintf(dst + n, "\\x%02x", c);
	}
	sprintf(dst + n, len > 60 ? "\"…" : "\"");
	return dst;
}

/* Byte-exact, so that a %c of '\0' cannot end the comparison early. */
void	compare(int ub, const char *fmt, const t_capture *ft, const t_capture *libc) {
	static int	ub_reported;
	char		f[256];
	char		e[256];
	char		g[256];
	ssize_t		i = 0;
	ssize_t		from;

	while (i < ft->len && i < libc->len && ft->buf[i] == libc->buf[i])
		i++;
	if ((i == ft->len && i == libc->len && ft->ret == libc->ret) || (ub && ub_reported++))
		return ;
	from = i > 20 ? i - 20 : 0;
	quote(f, fmt, strlen(fmt), 0);
	quote(e, libc->buf + from, libc->len - from, from);
	quote(g, ft->buf + from, ft->len - from, from);
	if (!ub)
		fail("format   %s\n  expected %s  ret %d\n  got      %s  ret %d", f, e, libc->ret, g, ft->ret);
	warning("undefined behaviour, glibc as reference (first difference shown):\n"
		"  format   %s\n  expected %s  ret %d\n  got      %s  ret %d", f, e, libc->ret, g, ft->ret);
}

/* Whether the C standard defines a "%<flags>[.]<conv>" conversion. */
int	defined(const char *flags, int precision, char conv) {
	if (strchr(flags, '#') && !strchr("xX", conv))
		return 0;
	if ((strchr(flags, '+') || strchr(flags, ' ')) && !strchr("di", conv))
		return 0;
	if (strchr(flags, '0') && !strchr("diuxX", conv))
		return 0;
	return !(precision && strchr("cp", conv));
}

static void	run_test(const t_test *t) {
	t->fn();
}

/* ═══════════════════════════════ fairy core ═══════════════════════════════ */
/* Identical in libft-fairy, printf-fairy and gnl-fairy: every test runs in a */
/* child process, so a crash or an infinite loop only fails that test. What   */
/* the child writes (fail and warning lines, sanitizer reports, glibc's own   */
/* "double free" messages) is shown under the test when it did not pass.      */

static int	g_count[3];	/* tests passed, failed, passed with warnings */
static int	g_warned;	/* in the child: warning() was called */
static int	g_timeout = TEST_TIMEOUT;	/* 1 second once a test hung */
int			g_lenient = 0;	/* outside the subject: a failure only warns */

void	die(const char *what) {
	perror(what);
	exit(EXIT_FAILURE);
}

static void	vreport(const char *tag, const char *fmt, va_list ap) {
	fprintf(stderr, "%s ", tag);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
}

/* Fails the running test: reason shown under it. */
void	fail(const char *fmt, ...) {
	va_list	ap;

	va_start(ap, fmt);
	vreport("↳", fmt, ap);
	va_end(ap);
	_exit(1);
}

/* Flags something debatable: the test still passes, the reason is shown. */
void	warning(const char *fmt, ...) {
	va_list	ap;

	va_start(ap, fmt);
	vreport("⚠", fmt, ap);
	va_end(ap);
	g_warned = 1;
}

/* Runs one test in a child: 0 passed, 1 failed, 2 warned. Its output in log. */
static int	fork_test(const t_test *t, char *log, size_t size) {
	static int	fd = -1;
	pid_t		pid;
	int			status;
	ssize_t		n;
	size_t		len;

	if (fd < 0) {
		FILE	*f = tmpfile();

		if (!f)
			die("fairy: tmpfile");
		fd = fileno(f);
	}
	if (ftruncate(fd, 0) < 0 || lseek(fd, 0, SEEK_SET) < 0)
		die("fairy: tmpfile");
	fflush(NULL);
	pid = fork();
	if (pid < 0)
		die("fairy: fork");
	if (!pid) {
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		alarm(g_timeout);
		run_test(t);
		_exit(g_warned ? 2 : 0);
	}
	if (waitpid(pid, &status, 0) < 0)
		die("fairy: waitpid");
	n = pread(fd, log, size - 1, 0);
	len = n > 0 ? (size_t)n : 0;
	log[len] = '\0';
	if (WIFEXITED(status))
		return WEXITSTATUS(status) == 2 ? 2 : WEXITSTATUS(status) != 0;
	if (WTERMSIG(status) != SIGALRM)
		snprintf(log + len, size - len, "↳ crashed: %s\n", strsignal(WTERMSIG(status)));
	else {
		snprintf(log + len, size - len, "↳ timeout after %ds (infinite loop?)%s\n", g_timeout,
			g_timeout == TEST_TIMEOUT ? "" : ", cut short since a test hung");
		g_timeout = 1;
	}
	return 1;
}

void	section(const char *title) {
	int	n = WIDTH - 4 - (int)strlen(title);

	printf("\n" DIM "──" RESET " " BOLD "%s" RESET " " DIM, title);
	for (int i = 0; i < (n > 3 ? n : 3); i++)
		fputs("─", stdout);
	puts(RESET);
}

/* Prints a test's output indented, without the "==pid==" prefixes, the "====="
 * rulers and the shadow memory map of sanitizer reports. */
static void	print_log(const char *log) {
	const char	*end;
	const char	*p;

	for (; *log; log = *end ? end + 1 : end) {
		end = strchr(log, '\n');
		if (!end)
			end = log + strlen(log);
		p = log + strspn(log, "=");
		if (p == end)
			continue ;
		if (p - log == 2 && isdigit(*p) && *(p += strspn(p, "0123456789")) == '=' && p[1] == '=')
			log = p + 2 + (p[2] == ' ');
		if (!strncmp(log, "Shadow bytes around", 19))
			return ;
		printf("     %.*s\n", (int)(end - log), log);
	}
}

/* Runs a group of tests; prints those that did not pass (all in VERBOSE). */
void	run_group(const char *title, const t_test *tests, size_t n) {
	static const char	*tags[] = {GREEN "[OK]", RED "[KO]", YELLOW "[OK]", YELLOW "[KO]"};
	static char			log[16384];
	int					shown = 0;
	int					res;

	for (size_t i = 0; i < n; i++) {
		res = fork_test(&tests[i], log, sizeof(log));
		if (res == 1 && g_lenient)
			res = 3;
		g_count[res == 3 ? 2 : res]++;
		if (res == 0 && !VERBOSE)
			continue ;
		if (!shown++)
			section(title);
		printf("%s" RESET " %s\n", tags[res], tests[i].name);
		print_log(log);
	}
}

/* Hands the counts to run.sh; returns the binary's exit status: 0, 1 or 2. */
int	fairy_summary(void) {
	const char	*path = getenv("FAIRY_STATS");
	FILE		*f = path ? fopen(path, "a") : NULL;

	if (f) {
		fprintf(f, "%d %d %d\n", g_count[0] + g_count[2], g_count[1], g_count[2]);
		fclose(f);
	}
	return g_count[1] ? 1 : g_count[2] ? 2 : 0;
}

/* ══════════════════════════════ end of core ═══════════════════════════════ */
