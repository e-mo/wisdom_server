#include <stdio.h>

#include "child.h"

int child_main(int client_fd, char *addr) {
	printf("child: hello from %s\n", addr);

	for (long i = 0; i < 1000000; i++);
}
