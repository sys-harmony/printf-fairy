#include "printf_fairy.h"
#include <stdint.h>

/*
 * Run under valgrind: any leak, invalid access or use of an uninitialised
 * value fails. Outputs range from a few bytes to 8 KB, so implementations
 * that switch from a stack buffer to malloc past some size are covered on
 * both paths. stdout goes to /dev/null to keep the report readable.
 */

static void	leak_test_short(void) {
	ft_printf("short %s %d %x\n", "hello", 42, 255);
}

static void	leak_test_long(void) {
	char	big[8192];

	memset(big, 'a', sizeof(big) - 1);
	big[sizeof(big) - 1] = '\0';
	ft_printf("big = %s\n", big);
}

static void	leak_test_many_args(void) {
	ft_printf("%c %s %p %d %i %u %x %X %%\n",
		'Z', "test", (void *)0xabcd, -1, INT32_MAX, 42u, 0xff, 0xABCD);
}

static void	leak_test_null_pointers(void) {
	ft_printf("%s %p\n", (char *)NULL, (void *)NULL);
}

int	main(void) {
	silence_stdout();
	leak_test_short();
	leak_test_long();
	leak_test_many_args();
	leak_test_null_pointers();
	return 0;
}
