#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>

#include "child.h"

#define ever ;;

#define BACKLOG_SIZE 100
#define errorf_exit(x, ...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(x); }

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);

int main(int argc, char *argv[]) {
	if (argc != 2) errorf_exit(1, "Usage: %s [port]\n", argv[0]);

	char *port = argv[1];

	struct addrinfo hints;
	memset(&hints, 0, (sizeof hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int rv;
	struct addrinfo *servinfo;
	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
		errorf_exit(1, "server: getaddrinfo: %s\n", gai_strerror(rv));

	struct addrinfo *p;
	int socket_fd;
	for (p = servinfo; p != NULL; p = p->ai_next) {

		socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socket_fd == -1) {
			perror("server: socket"); 
			continue;
		}

		int yes = 1;
		// int rv;
		rv = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, (sizeof yes));
		if (rv == -1) 
			errorf_exit(1, "server: setsockopt: %s\n", strerror(errno));

		if (bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_fd); 
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL) errorf_exit(1, "server: failed to bind socket\n");

	if (listen(socket_fd, BACKLOG_SIZE) == -1)
		errorf_exit(1, "server: listen: %s\n", strerror(errno));
	
	// Set up a handler to wait on child processes on sigchld
	struct sigaction sa; 
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		errorf_exit(1, "server: sigaction: %s\n", strerror(errno));


	printf("server: waiting for connections on port %s\n", port);

	int client_fd;
	struct sockaddr_storage client_addr;
	socklen_t sin_size;
	char addr[INET6_ADDRSTRLEN];
	for(ever) { // and ever, and ever

		sin_size = (sizeof client_addr);	
		client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &sin_size);
		if (client_fd == -1) {
			perror("server: accept");
			continue;
		}

		inet_ntop(
				client_addr.ss_family, 
				get_in_addr((struct sockaddr *) &client_addr),
				addr, 
				(sizeof addr)
		);
		printf("server: received connection\n\t%s\n", addr);

		// Every connection gets forked into its own process
		if (!fork()) { // This is a child <-> client connection
			close(socket_fd); // Child does not need the listener

			// Welcome to your new process, child
			child_main(client_fd, addr);
			exit(0);
		}
		
		// This socket lives on in a child process
		// so we can close it here.
		close(client_fd);
	}

	return 0;
}

// Reap the zombified children
void sigchld_handler(int s) {

	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
	if (!sa) return NULL;

	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in *)sa)->sin_addr);

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
