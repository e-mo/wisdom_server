#include <stdio.h>
#include <sys/socket.h>

#include "child.h"

int child_main(int client_fd, char *addr) {
	printf("child: hello from %s\n", addr);

	char *buf[100];
	size_t num;
	num = recv(client_fd, buf, 100, 0);	
	printf("%.*s", num, buf);
}
