#include "compare.h"
#include "fft.h"
#include <math.h>

// NEEDS TUNINGGG!!!!!!!!!
static const float ROTATION_WEIGHT = 1.0f;
static const float ACCEL_ENERGY_WEIGHT = 1.0f;
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

// helper to calculate total accelerometer motion energy for a gesture
static float accelEnergy(const Gesture_t &gesture)
{
    float energy = 0.0f;

    for (uint16_t i = 0; i < gesture.sample_count; i++)
    {
        float ax = gesture.samples[i].ax;
        float ay = gesture.samples[i].ay;
        float az = gesture.samples[i].az;
        float magnitude = sqrtf((ax * ax) + (ay * ay) + (az * az));
        float deviation = fabsf(magnitude - 1.0f);

        energy += deviation * deviation;
    }

    return energy;
}

// helper to build an accelerometer magnitude deviation signal and compute FFT-based features
static float accelDeviationFromSample(const IMUReading &sample)
{
    float magnitude = sqrtf((sample.ax * sample.ax) +
                            (sample.ay * sample.ay) +
                            (sample.az * sample.az));

    return fabsf(magnitude - 1.0f);
}

// builds an accelerometer magnitude deviation signal for FFT input
static bool buildAccelFftSignal(const Gesture_t &gesture, float signal[FFT_SIZE])
{
    if (gesture.sample_count == 0)
    {
        return false;
    }

    if (gesture.sample_count == 1)
    {
        float deviation = accelDeviationFromSample(gesture.samples[0]);

        for (int i = 0; i < FFT_SIZE; i++)
        {
            signal[i] = deviation;
        }

        return true;
    }

    for (int i = 0; i < FFT_SIZE; i++)
    {
        uint32_t sourceIndex =
            ((uint32_t)i * (gesture.sample_count - 1)) / (FFT_SIZE - 1);

        signal[i] = accelDeviationFromSample(gesture.samples[sourceIndex]);
    }

    return true;
}

static float fftErrorFromSamples(const Gesture_t &performed, const Gesture_t &recorded)
{
    float performedSignal[FFT_SIZE];
    float recordedSignal[FFT_SIZE];
    FFTFeaturesResult performedFft;
    FFTFeaturesResult recordedFft;

    if (!buildAccelFftSignal(performed, performedSignal) ||
        !buildAccelFftSignal(recorded, recordedSignal))
    {
        return 0.0f;
    }

    if (!FFT_Compute(performedSignal, &performedFft) ||
        !FFT_Compute(recordedSignal, &recordedFft))
    {
        return 0.0f;
    }

    return FFTFeaturesError(&performedFft, &recordedFft);
}

CompareResult Compare_Gestures(const Gesture_t &performed, const Gesture_t &recorded)
{
    CompareResult result;

    result.rotation_error = vectorError(performed.axes, recorded.axes);
    result.accel_energy_error = fabsf(accelEnergy(performed) - accelEnergy(recorded));
    result.peak_error = fabsf(performed.peak_vel - recorded.peak_vel);
    result.fft_error = fftErrorFromSamples(performed, recorded);

    // combine errors into total error with weights
    result.total_error =
        ROTATION_WEIGHT * result.rotation_error +
        ACCEL_ENERGY_WEIGHT * result.accel_energy_error +
        PEAK_WEIGHT * result.peak_error +
        FFT_WEIGHT * result.fft_error;

    // determine if matched based on threshold
    result.matched = result.total_error < MATCH_THRESHOLD;

    return result;
}
