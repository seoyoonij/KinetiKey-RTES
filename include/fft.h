#ifndef FFT_H
#define FFT_H

#include <stdint.h>

#define FFT_SIZE 256
#define FFT_FEATURES_SAMPLE_RATE_HZ 104.0f
#define FFT_FEATURES_NOISE_FLOOR 1.0f

static const int FFTFeaturesBinCount = 8;

typedef struct
{
    float peakFrequency;
    float peakMagnitude;
    float totalEnergy;
    float normalizedBins[FFTFeaturesBinCount];
} FFTFeaturesResult;

bool FFT_Init(void);

void FFT_Clear(FFTFeaturesResult *result);

bool FFT_Compute(const float *samples, FFTFeaturesResult *result);

float FFTFeaturesError(const FFTFeaturesResult *performed,
                       const FFTFeaturesResult *recorded);

#endif