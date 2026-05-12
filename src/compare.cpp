#include "compare.h"
#include "fft.h"
#include <math.h>
#include <stdio.h>

// NEEDS TUNINGGG!!!!!!!!!
static const float ROTATION_WEIGHT = 1.0f;
static const float ACCEL_ENERGY_WEIGHT = 1.0f;
static const float DURATION_WEIGHT = 20.0f;
static const float PEAK_WEIGHT = 0.5f;
static const float FFT_WEIGHT = 0.5f;
static const float MATCH_THRESHOLD = 60.0f;

// compares performed gesture to the recorded key
static float vectorError(const float a[3], const float b[3])
{
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float dz = a[2] - b[2];

    return sqrtf(dx * dx + dy * dy + dz * dz);
}

static float accelEnergyPerSecond(const Gesture_t &gesture)
{
    if (gesture.duration_ms == 0)
    {
        return 0.0f;
    }

    return gesture.accel_energy / ((float)gesture.duration_ms / 1000.0f);
}


CompareResult Compare_Gestures(const Gesture_t &performed, const Gesture_t &recorded)
{
    CompareResult result;

    result.rotation_error = vectorError(performed.axes, recorded.axes);
    result.accel_energy_error = fabsf(accelEnergyPerSecond(performed) -
                                      accelEnergyPerSecond(recorded));
    result.duration_error = fabsf((float)performed.duration_ms -
                                  (float)recorded.duration_ms) / 1000.0f;
    result.peak_error = fabsf(performed.peak_vel - recorded.peak_vel);
    if (performed.fft_valid && recorded.fft_valid)
    {
        result.fft_error = FFTFeaturesError(&performed.fft_features, &recorded.fft_features);
    }
    else
    {
        result.fft_error = 0.0f;
    }

    // combine errors into total error with weights
    result.total_error =
        ROTATION_WEIGHT * result.rotation_error +
        ACCEL_ENERGY_WEIGHT * result.accel_energy_error +
        DURATION_WEIGHT * result.duration_error +
        PEAK_WEIGHT * result.peak_error +
        FFT_WEIGHT * result.fft_error;

    // determine if matched based on threshold
    result.matched = result.total_error < MATCH_THRESHOLD;

    // ADDED DEBUG PRINT: displays compare components for threshold tuning
    printf("COMPARE rotation=%.2f accel_energy=%.2f duration=%.2f peak=%.2f fft=%.2f total=%.2f matched=%d\r\n",
           result.rotation_error,
           result.accel_energy_error,
           result.duration_error,
           result.peak_error,
           result.fft_error,
           result.total_error,
           (int)result.matched);

    return result;
}
