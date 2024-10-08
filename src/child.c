#include <stdlib.h>
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
#include "cbuffer.h"

#include "wisdom_inet.h"
#include "sensor_type.h"

enum _child_return {
	CHILD_OK,	
	CHILD_BUFFER_OVERFLOW
};

#define RECV_BUFFER_SIZE (100 * 1024 * 2)
#define TEMP_BUFFER_SIZE (100 * 1024)

int child_main(int client_fd, char *addr) {

	printf("Child starting!\n");
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	// 100kb
	cbuffer_t *recv_buf = cbuffer_create(RECV_BUFFER_SIZE);
	uint8_t temp_buf[TEMP_BUFFER_SIZE] = {0};

	bool new_packet = true;
	wisdom_header_t header;
	uint8_t *payload = NULL;
	uint16_t new_payload_size = 0;
	for (;;) {

		int num = recv(client_fd, temp_buf, TEMP_BUFFER_SIZE, 0);	
		if (num < 1) {
			if (num == 0) break;
			else if (errno != EAGAIN && errno != EWOULDBLOCK)
				break;

			usleep(1000 * 10);
			continue;
		}

		cbuffer_push(recv_buf, temp_buf, num);

		if (new_packet) {
			if (cbuffer_length(recv_buf) < WISDOM_HEADER_SIZE)
				continue;

			new_packet = false;

			cbuffer_pop(recv_buf, &header, WISDOM_HEADER_SIZE);
		}

		if (cbuffer_length(recv_buf) < payload_size(&header))
			continue;

		payload = malloc(payload_size(&header));
		cbuffer_pop(recv_buf, payload, payload_size(&header));

		switch(payload_type(&header)) {
		case WISDOM_PACKET_ECHO:
			printf("Echo packet\n");
			printf("%.*s\n", payload_size(&header), payload);

			break;
		case WISDOM_PACKET_SENSOR:
			printf("Sensor packet\n");	

			uint16_t sensor_type = SENSOR_TYPE(payload);
			switch (sensor_type) {
			case SHT30:
				printf("sht30\n");
				break;
			default:
				printf("sensor type unknown\n");
				break;
			}
		}

		new_packet = true;
		free(payload);
		payload == NULL;
	}

	printf("Child closing!\n");

	return CHILD_OK; // 0
}
