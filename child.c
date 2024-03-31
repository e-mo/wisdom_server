#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdbool.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "child.h"

enum _child_return {
	CHILD_OK,	
	CHILD_BUFFER_OVERFLOW
};

enum _header_index {
	PACKET_TYPE,
	PACKET_SIZE
};

enum _packet_types {
	ECHO_PACKET
};

#define RECV_BUFFER_SIZE (100 * 1024)

typedef uint32_t header_t;
#define HEADER_SIZE (sizeof (header_t))

#define packet_type(h) (((uint16_t *)h)[PACKET_TYPE])
#define packet_size(h) (((uint16_t *)h)[PACKET_SIZE])

int child_main(int client_fd, char *addr) {

	printf("Child starting!\n");
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	char buf[RECV_BUFFER_SIZE] = {0};
	header_t *header_p = (header_t *)buf;
	void *data_p = (void *)&buf[HEADER_SIZE];
	char *bp = buf;

	int num = 0;
	size_t received = 0;	
	bool new_packet = true;
	for (;;) {

		num = recv(client_fd, bp, RECV_BUFFER_SIZE - received, 0);	
		if (num < 1) {
			if (num == 0) break;
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				break;

			printf("would have blocked\n");
			usleep(1000 * 10);
			continue;
		}

		received += num;
		bp += num;

		// Close connection if we overflow.
		// TODO: Deal with my problems like an adult
		if (received >= RECV_BUFFER_SIZE) 
			return CHILD_BUFFER_OVERFLOW; // 1

		// If we haven't even recieved a header, wait for more 
		if (received < HEADER_SIZE) continue;
		
		if (new_packet) {
			new_packet = false;

			// process header here
			uint16_t *hp_16 = (uint16_t *)header_p;
			hp_16[PACKET_TYPE] = ntohs(hp_16[PACKET_TYPE]);
			hp_16[PACKET_SIZE] = ntohs(hp_16[PACKET_SIZE]);
		}

		if (received < packet_size(header_p)) continue;
		
		switch (packet_type(header_p)) {
		case ECHO_PACKET:	
			printf("%.*s\n", packet_size(header_p), (char *)data_p);
			break;
		}

		// reset buffer
		// TODO: This may actually erease received bytes 
		// if we recieved part of another packet.
		// FIX THIS
		bp = buf;
		received = 0;
		new_packet = true;
	}

	printf("Child closing!\n");

	return CHILD_OK; // 0
}
