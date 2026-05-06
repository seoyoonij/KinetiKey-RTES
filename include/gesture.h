#ifndef GESTURE_H
#define GESTURE_H

#include <stdint.h>
#include <stdbool.h>
#include "data_struct.h"

typedef struct
{
    float axes[3];        // [x,y,z] angles (degrees)
    float peak_vel;       // peak angular velocity magnitude (rad/s)
    uint32_t duration_ms; // duration of motion (ms)
} Gesture_t;

void Gesture_Reset(Gesture_t *g);
void Gesture_Update(Gesture_t *g, float gx, float gy, float gz, float dt);
float Gesture_Error(Gesture_t performed, Gesture_t recorded); // use Euclidean vector distance
bool isMotionDetected(const IMUReading &reading);

// Parameters
const float MOTION_THRESHOLD = 0.25f; // Adjust based on testing

#endif