#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdbool.h>
#include <time.h>

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
	printf("Child process started: %s\n", addr);
	fflush(stdout);

	// Set socket to non-blocking
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	cbuffer_t *recv_buf = cbuffer_create(RECV_BUFFER_SIZE);
	uint8_t temp_buf[TEMP_BUFFER_SIZE] = {0};

	bool new_packet = true;
	wisdom_header_t header;
	uint8_t *payload = NULL;
	uint16_t new_payload_size = 0;
	bool connection_closed = false;
	for (;;) {

		if (!connection_closed) {
			int num = recv(client_fd, temp_buf, TEMP_BUFFER_SIZE, 0);	
			if (num < 1) {
				if (num == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
					printf("Connection closed: %s\n", addr);
					connection_closed = true;
					continue;
				}

				usleep(1000 * 10);
				continue;
			}

			cbuffer_push(recv_buf, temp_buf, num);
		}

		// Continues until a full header has been received, and then pops it off
		// the buffer.
		// If the connection has closed and not enough data has been received
		// to construct at least a header, discard the buffer and stop child process.
		if (new_packet) {
			if (cbuffer_length(recv_buf) < WISDOM_HEADER_SIZE) {
				if (connection_closed) break;
				continue;
			}

			new_packet = false;
			cbuffer_pop(recv_buf, &header, WISDOM_HEADER_SIZE);
		}

		// Temporary variable to hold date time stamp packed data size
		uint date_time_size = 5;

		// Now that we have a header we know how many bytes we are expecting.
		// Until we have recieved that many bytes, continue.
		// If we find the connection closed and we can't construct a complete payload,
		// discard the buffer and stop child process.
		if (cbuffer_length(recv_buf) < payload_size(&header)) { // + date_time_size) {
			if (connection_closed) break;
			continue;
		}

		// We have enough for a full payload.
		// Pyp is off the stack + timestamp that follows every packet.
		payload = malloc(payload_size(&header) + date_time_size);
		cbuffer_pop(recv_buf, payload, payload_size(&header)); // + date_time_size);

		// Timestamp
		time_t t = time(NULL); 
		struct tm tm = *localtime(&t);
		printf("%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

		printf("Packet type: ");
		switch(payload_type(&header)) {
		case WISDOM_PACKET_ECHO:
			printf("echo\n");
			printf("Contents: %uB\n", payload_size(&header));
			printf("%.*s\n", payload_size(&header), payload);

			break;
		case WISDOM_PACKET_SENSOR:
			printf("sensor\n");

			uint16_t sensor_type = SENSOR_TYPE(payload);
			printf("type: %u\n", sensor_type);
			switch (sensor_type) {
			case SHT30:
				printf("sht30\n");
				uint8_t *pp = payload;
				pp += sizeof (uint16_t);
				float temp = *(float *)pp;
				pp += sizeof (float);
				float humid = *(float *)pp;
				pp += sizeof (float);
				// At end, now print time
				uint8_t min = *pp++;
				uint8_t hour = *pp++;
				uint8_t day = *pp++;
				uint8_t month = *pp++;
				uint8_t year = *pp;
				printf("t: %f\nh: %f\n", temp, humid);
				printf("%02u:%02u %02u.%02u.%02u\n", hour, min, year, month, day);
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

	printf("Child process stopped: %s\n\n", addr);
	fflush(stdout);

	return CHILD_OK; // 0
}
