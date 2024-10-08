#ifndef WISDOM_INET_H
#define WISDOM_INET_H

enum WISDOM_HEADER_INDEX {
	WISDOM_PACKET_TYPE,
	WISDOM_PACKET_SIZE
};

enum WISDOM_PACKET_TYPE {
	WISDOM_PACKET_ECHO,
	WISDOM_PACKET_SENSOR
};

typedef uint32_t wisdom_header_t;
#define WISDOM_HEADER_SIZE (sizeof (wisdom_header_t))

#define payload_type(h) (((uint16_t *)h)[WISDOM_PACKET_TYPE])
#define payload_size(h) (((uint16_t *)h)[WISDOM_PACKET_SIZE])

#endif // WISDOM_INET_H
