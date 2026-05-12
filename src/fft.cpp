#include "fft.h"
#include "arm_math.h"

static arm_rfft_fast_instance_f32 fftInstance;
static bool fftReady = false;

static float fftInput[FFT_SIZE];
static float fftOutput[FFT_SIZE];
static float fftMagnitude[FFT_SIZE / 2];

static float absFloat(float x)
{
    return (x < 0.0f) ? -x : x;
}

// Initializes the FFT instance
bool FFT_Init(void)
{
    arm_status status = arm_rfft_fast_init_f32(&fftInstance, FFT_SIZE);

    if (status == ARM_MATH_SUCCESS)
    {
        fftReady = true;
        return true;
    }

    fftReady = false;
    return false;
}

// Clears FFT features result
void FFT_Clear(FFTFeaturesResult *result)
{
    if (result == nullptr)
    {
        return;
    }

    result->peakFrequency = 0.0f;
    result->peakMagnitude = 0.0f;
    result->totalEnergy = 0.0f;

    for (int i = 0; i < FFTFeaturesBinCount; i++)
    {
        result->normalizedBins[i] = 0.0f;
    }
}

// Computes FFT features from input samples
bool FFT_Compute(const float *samples, FFTFeaturesResult *result)
{
    if (!fftReady || samples == nullptr || result == nullptr)
    {
        return false;
    }

    FFT_Clear(result);

    float mean = 0.0f;

    for (int i = 0; i < FFT_SIZE; i++)
    {
        mean += samples[i];
    }

    mean /= FFT_SIZE;

    for (int i = 0; i < FFT_SIZE; i++)
    {
        fftInput[i] = samples[i] - mean;
    }

    // Perform FFT
    arm_rfft_fast_f32(&fftInstance, fftInput, fftOutput, 0);
    arm_cmplx_mag_f32(fftOutput, fftMagnitude, FFT_SIZE / 2);

    // Extract features from FFT output
    float maxMagnitude = 0.0f;
    int maxBin = 0;
    float totalEnergy = 0.0001f;

    for (int bin = 1; bin < FFT_SIZE / 2; bin++)
    {
        float energy = fftMagnitude[bin] * fftMagnitude[bin];

        totalEnergy += energy;

        if (fftMagnitude[bin] > maxMagnitude)
        {
            maxMagnitude = fftMagnitude[bin];
            maxBin = bin;
        }
    }

    // set features in result
    result->peakMagnitude = maxMagnitude;
    result->totalEnergy = totalEnergy;

    if (maxMagnitude > FFT_FEATURES_NOISE_FLOOR)
    {
        result->peakFrequency =
            maxBin * (FFT_FEATURES_SAMPLE_RATE_HZ / FFT_SIZE);
    }
    else
    {
        result->peakFrequency = 0.0f;
    }

    // compute normalized energy in FFT bins for feature
    float featureEnergy = 0.0001f;

    for (int i = 0; i < FFTFeaturesBinCount; i++)
    {
        int bin = i + 1;
        float energy = fftMagnitude[bin] * fftMagnitude[bin];

        result->normalizedBins[i] = energy;
        featureEnergy += energy;
    }

    for (int i = 0; i < FFTFeaturesBinCount; i++)
    {
        result->normalizedBins[i] /= featureEnergy;
    }

    return true;
}

// Computes error between two FFT feature sets
float FFTFeaturesError(const FFTFeaturesResult *performed,
                       const FFTFeaturesResult *recorded)
{
    if (performed == nullptr || recorded == nullptr)
    {
        return 9999.0f;
    }

    float binErrorSquared = 0.0f;

    for (int i = 0; i < FFTFeaturesBinCount; i++)
    {
        float diff =
            performed->normalizedBins[i] - recorded->normalizedBins[i];

        binErrorSquared += diff * diff;
    }

    // compute bin error as root mean square of bin differences
    float binError = 0.0f;
    arm_sqrt_f32(binErrorSquared, &binError);

    float frequencyError =
        absFloat(performed->peakFrequency - recorded->peakFrequency);

    float energyError = 0.0f;

    // avoid divide by zero
    if (recorded->totalEnergy > 0.001f)
    {
        energyError =
            absFloat(performed->totalEnergy - recorded->totalEnergy) /
            recorded->totalEnergy;
    }

    // combine errors with weights into total error
    return 100.0f * binError
         + 2.0f * frequencyError
         + 10.0f * energyError;
}
