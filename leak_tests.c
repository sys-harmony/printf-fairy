#include "printf_fairy.h"
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>

/*
 * ft_vdprintf alloue dynamiquement quand le format depasse PRINTF_BUFFER_SIZE.
 * On force le chemin malloc avec une grosse string, puis on relance avec un
 * format court pour couvrir le chemin stack buffer.
 * stdout est redirige vers /dev/null pour ne pas polluer la sortie.
 */

static void	redirect_stdout_to_null(void) {
	int	fd = open("/dev/null", O_WRONLY);

	if (fd < 0)
		error_exit("printf-fairy: open /dev/null");
	if (dup2(fd, 1) < 0)
		error_exit("printf-fairy: dup2");
	close(fd);
}

static void	leak_test_short(void) {
	ft_printf("short %s %d %x\n", "hello", 42, 255);
}

static void	leak_test_long(void) {
	char	big[8192];

	for (size_t i = 0; i < sizeof(big) - 1; i++)
		big[i] = 'a';
	big[sizeof(big) - 1] = 0;
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
	redirect_stdout_to_null();
	leak_test_short();
	leak_test_long();
	leak_test_many_args();
	leak_test_null_pointers();
	return 0;
}
