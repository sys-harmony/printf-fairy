#include "printf_fairy.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

static void	redirect_stdout_to_null(void) {
	int	fd = open("/dev/null", O_WRONLY);

	if (fd < 0)
		error_exit("printf-fairy: open /dev/null");
	if (dup2(fd, 1) < 0)
		error_exit("printf-fairy: dup2");
	close(fd);
}

static void	leak_test_flags_short(void) {
	ft_printf("%+05d %#10x %-5s\n", 42, 255, "hi");
}

static void	leak_test_precision_truncate(void) {
	ft_printf("%.3s %.5d\n", "abcdefghij", 42);
}

static void	leak_test_precision_zero(void) {
	ft_printf("%.0d %.0s\n", 0, "hello");
}

static void	leak_test_huge_padding(void) {
	ft_printf("[%5000d]\n", 42);
}

static void	leak_test_huge_string_with_width(void) {
	char	big[8192];

	for (size_t i = 0; i < sizeof(big) - 1; i++)
		big[i] = 'b';
	big[sizeof(big) - 1] = 0;
	ft_printf("[%-9000s]\n", big);
}

int	main(void) {
	redirect_stdout_to_null();
	leak_test_flags_short();
	leak_test_precision_truncate();
	leak_test_precision_zero();
	leak_test_huge_padding();
	leak_test_huge_string_with_width();
	return 0;
}
