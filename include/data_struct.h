#include <cstdint>
#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

typedef struct
{
    float ax, ay, az; // accelerometer reading
    float gx, gy, gz; // gyroscope reading

    uint32_t timestamp_ms;
} IMUReading; // : characterizes one sample type

#endif