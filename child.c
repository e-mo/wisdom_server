#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "child.h"

int child_main(int client_fd, char *addr) {


	char buf[100];
	size_t num;
	for (;;) {

		num = recv(client_fd, buf, 100, 0);	
		if (num < 1) break;
		buf[num] = '\0';

	}


	printf("Child closing!\n");
}
