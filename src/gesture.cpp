#include "gesture.h"
#include <math.h>
#include <string.h>

static GestureSeparationConfig g_cfg;
static GestureCaptureState s_cap_state = CAPTURE_WAITING;
static Gesture_t s_current;
static uint32_t s_motion_start_ms = 0;
static uint32_t s_still_start_ms = 0;

// === CHARACTERIZING GESTURE ===
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

float Gesture_Error(const Gesture_t &performed, const Gesture_t &recorded)
{
    // Euclidean distance between the two 3D points
    float dx = performed.axes[0] - recorded.axes[0];
    float dy = performed.axes[1] - recorded.axes[1];
    float dz = performed.axes[2] - recorded.axes[2];
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

// === SEPARATION CONFIG ===
void GestureSeparation_SetDefault(void)
{
    g_cfg.accel_start_threshold = 0.18f;
    g_cfg.accel_stop_threshold = 0.08f;
    g_cfg.gyro_start_threshold = 35.0f;
    g_cfg.gyro_stop_threshold = 15.0f;
    g_cfg.still_time_ms = 180;
    g_cfg.min_motion_ms = 120;
    g_cfg.max_motion_ms = 5000;
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

// === MOTION DETECT HELPERS ===
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

// === SEPARATION LOGIC (FSM) ===
// Clear FSM and capture buffer
void GestureCapture_Reset(void)
{
    s_cap_state = CAPTURE_WAITING;
    memset(&s_current, 0, sizeof(s_current));
    s_motion_start_ms = 0;
    s_still_start_ms = 0;
}
// Return current capture phase
GestureCaptureState GestureCapture_GetState(void)
{
    return s_cap_state;
}
// Core gesture FSM
bool GestureCapture_Update(const IMUReading *sample, Gesture_t *out_gesture)
{
    if (!sample || !out_gesture)
        return false;
    GestureSeparationConfig cfg = GestureSeparation_GetConfig();

    bool moving = Gesture_IsMotionInstant(sample->ax, sample->ay, sample->az, sample->gx, sample->gy, sample->gz);
    uint32_t now = sample->timestamp_ms;

    switch (s_cap_state)
    {
    case CAPTURE_WAITING: // wait for movement
        if (moving)
        {
            memset(&s_current, 0, sizeof(s_current));
            s_current.start_time_ms = now;
            s_motion_start_ms = now;
            s_still_start_ms = 0;
            s_cap_state = CAPTURING;
        }
        return false;
    case CAPTURING: // collect samples into capture buffer
        if (s_current.sample_count < gesture_capacity())
        {
            s_current.samples[s_current.sample_count++] = *sample;
        }
        // Time duration threshold: too long, force complete
        if ((now - s_motion_start_ms) >= cfg.max_motion_ms)
        {
            s_current.end_time_ms = now;
            *out_gesture = s_current;
            GestureCapture_Reset();
            return true;
        }
        if (!moving)
        {
            s_still_start_ms = now;
            s_cap_state = CAPTURE_END_CANDIDATE;
        }
        return false;
    case CAPTURE_END_CANDIDATE: // when still enough for long enough
        if (s_current.sample_count < gesture_capacity())
        {
            s_current.samples[s_current.sample_count++] = *sample;
        }
        // motion resumed. continue capturing
        if (moving)
        {
            s_still_start_ms = 0;
            s_cap_state = CAPTURING;
            return false;
        }
        // no motion for long enough. complete capture
        if ((now - s_still_start_ms) >= cfg.still_time_ms)
        {
            uint32_t dur = now - s_motion_start_ms;
            // if too short or noisy, discard
            if (dur < cfg.min_motion_ms)
            {
                GestureCapture_Reset();
                return false;
            }
            s_current.end_time_ms = now;
            *out_gesture = s_current;
            GestureCapture_Reset();
            return true;
        }
        return false;
    }
    return false;
}