#ifndef GESTURE_H
#define GESTURE_H

#include <stdint.h>
#include <stdbool.h>
#include "data_struct.h"
#include "fft.h"

typedef struct
{
    // capture buffer info
    uint16_t sample_count;
    uint32_t start_time_ms;
    uint32_t end_time_ms;
    IMUReading first_sample;
    IMUReading last_sample;

    // additional info
    float axes[3];          // [x,y,z] angles (degrees)
    float peak_vel;         // peak angular velocity magnitude (deg/s)
    float accel_energy;     // accumulated accelerometer motion energy
    float peak_accel_dev_g; // largest accel deviation from 1g
    FFTFeaturesResult fft_features;
    bool fft_valid;
    uint32_t duration_ms;   // duration of motion (ms)
} Gesture_t; // : holds compact features that describe one gesture

void Gesture_Reset(Gesture_t *g);
void Gesture_Update(Gesture_t *g, float gx, float gy, float gz, float dt);
float Gesture_Error(const Gesture_t &performed, const Gesture_t &recorded); // use Euclidean vector distance

// "how to separate one gesture"
typedef struct
{
    float accel_start_threshold;
    float accel_stop_threshold;
    float gyro_start_threshold;
    float gyro_stop_threshold;
    uint32_t still_time_ms;
    uint32_t min_motion_ms;
    uint32_t max_motion_ms;
} GestureSeparationConfig;

// Safe defaults (tune later)
void GestureSeparation_SetDefault(void);
void GestureSeparation_SetConfig(const GestureSeparationConfig *cfg);
GestureSeparationConfig GestureSeparation_GetConfig(void);

// Intensity helpers for verification
float Gesture_AccelDeviationG(float ax, float ay, float az);  // | |a| - 1g |
float Gesture_GyroMagnitudeDps(float gx, float gy, float gz); // sqrt(gx^2+gy^2+gz^2)
bool Gesture_IsMotionInstant(float ax, float ay, float az, float gx, float gy, float gz);

// --- FOR ONE GESTURE ---
// Internal gesture states: later main converts this into GestureResult using States_HandleGestureComplete()
typedef enum
{
    CAPTURE_WAITING = 0,
    CAPTURING,
    CAPTURE_END_CANDIDATE
} GestureCaptureState;

// Regarding one IMU sample
void GestureCapture_Reset(void);
bool GestureCapture_Update(const IMUReading *sample, Gesture_t *out_gesture); // true: one full gesture is separated and copied to out_gesture
GestureCaptureState GestureCapture_GetState(void);

// Parameters
const float MOTION_THRESHOLD = 0.25f; // Adjust based on testing

#endif
