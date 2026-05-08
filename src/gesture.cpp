#include "gesture.h"
#include <math.h>

void Gesture_Reset(Gesture_t *g)
{
    g->axes[0] = 0.0f;
    g->axes[1] = 0.0f;
    g->axes[2] = 0.0f;
    g->peak_vel = 0.0f;
    g->duration_ms = 0;
}

void Gesture_Update(Gesture_t *g, float gx, float gy, float gz, float dt)
{
    // Integrate Angular Displacement
    g->axes[0] += gx * dt;
    g->axes[1] += gy * dt;
    g->axes[2] += gz * dt;

    // Track Intensity
    float current_vel = sqrtf(gx * gx + gy * gy + gz * gz);
    if (current_vel > g->peak_vel)
    {
        g->peak_vel = current_vel; // peak magnitude
    }

    // Track Time
    g->duration_ms += (uint32_t)(dt * 1000.0f);
}

float Gesture_Error(Gesture_t performed, Gesture_t recorded)
{
    // Euclidean distance between the two 3D points
    float dx = performed.axes[0] - recorded.axes[0];
    float dy = performed.axes[1] - recorded.axes[1];
    float dz = performed.axes[2] - recorded.axes[2];

    return sqrtf(dx * dx + dy * dy + dz * dz);
}

bool isMotionDetected(const IMUReading &reading)
{
    // Calculate magnitude: sqrt(x^2 + y^2 + z^2)
    float a_magnitude = sqrt(reading.ax * reading.ax + reading.ay * reading.ay + reading.az * reading.az);
    // float g_magnitude = sqrt(reading.gx * reading.gx + reading.gy * reading.gy + reading.gz * reading.gz);

    // Deviation from 1G (gravity)
    float a_deviation = fabsf(a_magnitude - 1.0f);

    return (a_deviation > MOTION_THRESHOLD);
}