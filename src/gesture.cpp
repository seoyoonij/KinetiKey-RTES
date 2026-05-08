#include "gesture.h"
#include <math.h>
#include <string.h>

static GestureSeparationConfig g_cfg;
static GestureCaptureState s_cap_state = CAPTURE_WAITING;
static Gesture_t s_current;
static uint32_t s_motion_start_ms = 0;
static uint32_t s_still_start_ms = 0;

static uint16_t gesture_capacity(void)
{
    return (uint16_t)(sizeof(s_current.samples) / sizeof(s_current.samples[0]));
}

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

void GestureSeparation_SetDefault(void)
{
    g_cfg.accel_start_threshold = 0.18f;
    g_cfg.accel_stop_threshold = 0.08f;
    g_cfg.gyro_start_threshold = 35.0f;
    g_cfg.gyro_stop_threshold = 15.0f;
    g_cfg.still_time_ms = 180;
    g_cfg.min_motion_ms = 120;
    g_cfg.max_motion_ms = 2000;
}

void GestureSeparation_SetConfig(const GestureSeparationConfig *cfg)
{
    if (cfg)
    {
        memcpy(&g_cfg, cfg, sizeof(GestureSeparationConfig));
    }
}
GestureSeparationConfig GestureSeparation_GetConfig(void)
{
    return g_cfg;
}

float Gesture_AccelDeviationG(float ax, float ay, float az)
{
    float amag = sqrtf(ax * ax + ay * ay + az * az);
    return fabsf(amag - 1.0f);
}
float Gesture_GyroMagnitudeDps(float gx, float gy, float gz)
{
    return sqrtf(gx * gx + gy * gy + gz * gz);
}
bool Gesture_IsMotionInstant(float ax, float ay, float az, float gx, float gy, float gz)
{
    float a_dev = Gesture_AccelDeviationG(ax, ay, az);
    float g_mag = Gesture_GyroMagnitudeDps(gx, gy, gz);
    // Start condition: either accel OR gyro crosses start threshold
    return (a_dev >= g_cfg.accel_start_threshold) || (g_mag >= g_cfg.gyro_start_threshold);
}

void GestureCapture_Reset(void)
{
    s_cap_state = CAPTURE_WAITING;
    memset(&s_current, 0, sizeof(s_current));
    s_motion_start_ms = 0;
    s_still_start_ms = 0;
}
GestureCaptureState GestureCapture_GetState(void)
{
    return s_cap_state;
}
bool GestureCapture_Update(const IMUReading *sample, Gesture_t *out_gesture)
{
    if (!sample || !out_gesture)
        return false;
    GestureSeparationConfig cfg = GestureSeparation_GetConfig();

    bool moving = Gesture_IsMotionInstant(sample->ax, sample->ay, sample->az, sample->gx, sample->gy, sample->gz);
    uint32_t now = sample->timestamp_ms;

    switch (s_cap_state)
    {
    case CAPTURE_WAITING:
        // TODO: if (moving) { start time, switch state}
        return false;
    case CAPTURING:
        // TODO: time duration threshold, motion threshold
        return false;
    case CAPTURE_END_CANDIDATE:
        // TODO: motion threshold, time threshold
        return false;
    }
    return false;
}