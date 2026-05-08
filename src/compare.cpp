#include "compare.h"
#include <math.h>

// NEEDS TUNINGGG!!!!!!!!!
static const float ROTATION_WEIGHT = 1.0f;
static const float ACCEL_WEIGHT = 1.0f; 
static const float PEAK_WEIGHT = 1.0f;
static const float FFT_WEIGHT = 1.0f;
static const float MATCH_THRESHOLD = 50.0f;

// compares performed gesture to the recorded key
static float vectorError(const float a[3], const float b[3])
{
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float dz = a[2] - b[2];

    return sqrtf(dx * dx + dy * dy + dz * dz);
}

CompareResult Compare_Gestures(const Gesture_t &performed, const Gesture_t &recorded)
{
    CompareResult result;

    // TODO: depends on gesture features we extract
    result.rotation_error = vectorError(performed.rotation, recorded.rotation);
    result.accel_error = vectorError(performed.accel_avg, recorded.accel_avg);
    result.peak_error = fabsf(performed.gyro_peak - recorded.gyro_peak);
    result.fft_error = 0.0f; // TODO: depends on fft.cpp

    // combine errors into total error with weights
    result.total_error =
        ROTATION_WEIGHT * result.rotation_error +
        ACCEL_WEIGHT * result.accel_error +
        PEAK_WEIGHT * result.peak_error +
        FFT_WEIGHT * result.fft_error;

    // determine if matched based on threshold
    result.matched = result.total_error < MATCH_THRESHOLD;

    return result;
}
