#include "printf_fairy.h"

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

	memset(big, 'b', sizeof(big) - 1);
	big[sizeof(big) - 1] = '\0';
	ft_printf("[%-9000s]\n", big);
}

int	main(void) {
	silence_stdout();
	leak_test_flags_short();
	leak_test_precision_truncate();
	leak_test_precision_zero();
	leak_test_huge_padding();
	leak_test_huge_string_with_width();
	return 0;
}
