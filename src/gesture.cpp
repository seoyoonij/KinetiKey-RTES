#include "gesture.h"
#include <math.h>
#include <string.h>

static GestureSeparationConfig g_cfg;
static GestureCaptureState s_cap_state = CAPTURE_WAITING;
static Gesture_t s_current;
static uint32_t s_motion_start_ms = 0;
static uint32_t s_still_start_ms = 0;
static uint32_t s_last_sample_ms = 0;
static float s_fft_signal[FFT_SIZE];
static uint16_t s_fft_sample_count = 0;
static float s_last_fft_value = 0.0f;

// resets gesture data structure
void Gesture_Reset(Gesture_t *g)
{
    if (!g)
    {
        return;
    }

    memset(g, 0, sizeof(Gesture_t));
}

// updates gesture features with new IMU sample (called in capture)
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

// computes error between two gestures as Euclidean distance between their angle vectors
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
    g_cfg.still_time_ms = 1000;
    g_cfg.min_motion_ms = 1000;
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
    Gesture_Reset(&s_current);
    s_motion_start_ms = 0;
    s_still_start_ms = 0;
    s_last_sample_ms = 0;
    s_fft_sample_count = 0;
    s_last_fft_value = 0.0f;
}
// Return current capture phase
GestureCaptureState GestureCapture_GetState(void)
{
    return s_cap_state;
}

// add new sample to capture buffer and update features, called on each IMU sample during capture
static void GestureCapture_AddSample(const IMUReading *sample)
{
    uint32_t now = sample->timestamp_ms;
    float dt = 0.0f;
    float accel_dev = Gesture_AccelDeviationG(sample->ax, sample->ay, sample->az);

    if (s_last_sample_ms != 0 && now > s_last_sample_ms)
    {
        dt = (float)(now - s_last_sample_ms) / 1000.0f;
    }

    Gesture_Update(&s_current, sample->gx, sample->gy, sample->gz, dt);
    s_last_sample_ms = now;

    if (s_current.sample_count == 0)
    {
        s_current.first_sample = *sample;
    }

    s_current.last_sample = *sample;
    s_current.sample_count++;
    s_current.accel_energy += accel_dev * accel_dev;

    if (accel_dev > s_current.peak_accel_dev_g)
    {
        s_current.peak_accel_dev_g = accel_dev;
    }

    if (s_fft_sample_count < FFT_SIZE)
    {
        s_fft_signal[s_fft_sample_count++] = accel_dev;
        s_last_fft_value = accel_dev;
    }
}

// called when motion ends to finalize gesture features and output completed gesture
static void GestureCapture_Finish(uint32_t end_time_ms, Gesture_t *out_gesture)
{
    s_current.end_time_ms = end_time_ms;
    s_current.duration_ms = end_time_ms - s_current.start_time_ms;

    if (s_fft_sample_count > 0)
    {
        for (uint16_t i = s_fft_sample_count; i < FFT_SIZE; i++)
        {
            s_fft_signal[i] = s_last_fft_value;
        }

        s_current.fft_valid = FFT_Compute(s_fft_signal, &s_current.fft_features);
    }

    *out_gesture = s_current;
    GestureCapture_Reset();
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
            Gesture_Reset(&s_current);
            s_current.start_time_ms = now;
            s_motion_start_ms = now;
            s_still_start_ms = 0;
            s_last_sample_ms = now;
            GestureCapture_AddSample(sample);
            s_cap_state = CAPTURING;
        }
        return false;
    case CAPTURING: // collect samples into capture buffer
        GestureCapture_AddSample(sample);

        // Time duration threshold: too long, force complete
        if ((now - s_motion_start_ms) >= cfg.max_motion_ms)
        {
            GestureCapture_Finish(now, out_gesture);
            return true;
        }
        if (!moving)
        {
            s_still_start_ms = now;
            s_cap_state = CAPTURE_END_CANDIDATE;
        }
        return false;
    case CAPTURE_END_CANDIDATE: // when still enough for long enough
        GestureCapture_AddSample(sample);

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
            GestureCapture_Finish(now, out_gesture);
            return true;
        }
        return false;
    }
    return false;
}
