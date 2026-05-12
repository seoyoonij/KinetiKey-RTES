#ifndef COMPARE_H
#define COMPARE_H

#include "gesture.h"

typedef struct
{
    float total_error; // overall error metric combining all factors
    float rotation_error; // error in rotation angles
    float accel_energy_error; // error in total accelerometer motion energy
    float duration_error; // error in gesture duration
    float peak_error; // error in peak gyro magnitude
    float fft_error; // error in frequency domain features
    bool matched; // matched or not based on threshold
} CompareResult;

CompareResult Compare_Gestures(
    const Gesture_t &performed,
    const Gesture_t &recorded
);

#endif
