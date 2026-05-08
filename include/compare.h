#ifndef COMPARE_H
#define COMPARE_H

#include "gesture.h"

typedef struct
{
    float total_error; // overall error metric combining all factors
    float rotation_error; // error in rotation angles (degrees)
    float accel_error; // error in peak angular velocity (rad/s)
    float intensity_error; // error in overall motion intensity
    float fft_error; // error in frequency domain features
    bool matched; // matched or not based on threshold
} CompareResult;

CompareResult Compare_Gestures(
    const Gesture_t &performed,
    const Gesture_t &recorded
);

#endif
