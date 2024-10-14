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
#include "ts_printf.h"

#include "wisdom_inet.h"
#include "sensor_type.h"

enum _child_return {
	CHILD_OK,	
	CHILD_BUFFER_OVERFLOW
};

#define RECV_BUFFER_SIZE (100 * 1024 * 2)
#define TEMP_BUFFER_SIZE (100 * 1024)

// uin8_t tp[5] = [min, hour, day, month, year]
void print_time(uint8_t *tp) {
		ts_printf("time: %02u:%02u %02u.%02u.%02u\n", tp[1], tp[0], tp[4], tp[3], tp[2]);
// Coo Code: ssssssssssssssssssssssssssssssssssssssssssssssss);
}

void process_sensor(uint8_t *payload) {
	uint16_t sensor_type = SENSOR_TYPE(payload);
	ts_printf("type: %u\n", sensor_type);

	switch (sensor_type) {
	case SHT30:

		ts_printf("sht30\n");
		payload += sizeof (uint16_t);
		float temp = *(float *)payload;
		payload += sizeof (float);
		float humid = *(float *)payload;
		payload += sizeof (float);
		ts_printf("temperature: %f\nhumidity: %f\n", temp, humid);
		break;

	default:
		ts_printf("sensor type unknown\n");
		return;
	}

	print_time(payload);
}

int child_main(int client_fd, char *addr) {
	ts_printf("Child process started: %s\n", addr);
	fflush(stdout);

	// Set socket to non-blocking
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	cbuffer_t *recv_buf = cbuffer_create(RECV_BUFFER_SIZE);
	uint8_t temp_buf[TEMP_BUFFER_SIZE] = {0};

	bool new_packet = true;
	wisdom_header_t header;
	// Max possible size (16 bit size);
	uint8_t payload[0xFFFF] = {0};
	bool connection_closed = false;
	for (;;) {

		if (!connection_closed) {
			int num = recv(client_fd, temp_buf, TEMP_BUFFER_SIZE, 0);	
			if (num < 1) {
				if (num == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
					ts_printf("Connection closed: %s\n", addr);
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
		cbuffer_pop(recv_buf, payload, payload_size(&header)); // + date_time_size);

		// Timestamp
		time_t t = time(NULL); 
		struct tm tm = *localtime(&t);

		ts_printf("Packet received\n");
		ts_printf("Type: ");
		switch(payload_type(&header)) {
		case WISDOM_PACKET_ECHO: 
			printf("echo\n");
			ts_printf("Size: %u\n", payload_size(&header));
			ts_printf("Payload: %.*s\n", payload_size(&header), payload);
			break;
		case WISDOM_PACKET_SENSOR:
			printf("sensor\n");
			process_sensor(payload);
		}

		new_packet = true;
	}

	ts_printf("Child process stopped: %s\n\n", addr);
	fflush(stdout);

	return CHILD_OK; // 0
}
