#ifndef WISDOM_SENSOR_TYPE_H
#define WISDOM_SENSOR_TYPE_H

typedef enum sensor_types {
	SHT30,
	SENSOR_TYPE_MAX // Keep at end
} SENSOR_TYPE_T;

#define SENSOR_TYPE(h) (*(uint16_t *)h) 

#endif // WISDOM_SENSOR_TYPE_H
