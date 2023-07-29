#include <stdio.h>
#include <sys/socket.h>

#include "child.h"

int child_main(int client_fd, char *addr) {
	printf("child: hello from %s\n", addr);

	char *buf[100];
	size_t num;

	for (;;) {

		num = recv(client_fd, buf, 100, 0);	
		if (num < 1) break;

		printf("%.*s", num, buf);
		printf("-\n");
	}
	printf("Child closing!\n");
}
