#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "child.h"

int child_main(int client_fd, char *addr) {
	printf("child: hello from %s\n", addr);

	char buf[100];
	size_t num;

	for (;;) {

		num = recv(client_fd, buf, 100, 0);	
		if (num < 1) break;
		buf[num] = '\0';

		if(strstr(buf, "PING"))
			send(client_fd, "PONG", 4, 0);

	}
	printf("Child closing!\n");
}
