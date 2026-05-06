#include <cstdint>
#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

typedef struct
{
    float x;
    float y;
    float z;
    uint32_t timestamp_ms;
} IMUReading;

#endif