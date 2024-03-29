#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "child.h"

int child_main(int client_fd, char *addr) {

	printf("Child starting!\n");
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	char buf[100];
	size_t num;

	for (;;) {

		num = recv(client_fd, buf, 100, 0);	
		if (num < 1) {
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				break;

			printf("would have blocked\n");
			usleep(1000 * 10);
			continue;
		}
	}

	printf("Child closing!\n");
}
