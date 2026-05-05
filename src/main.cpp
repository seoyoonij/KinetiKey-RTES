#include "mbed.h"

#include "gesture.h"
#include "imu_interface.h"
#include "states.h"

/**
TODO:

=== File structure===
GESTURE: vectors and math
STATES: IDLE, RECORDING, UNLOCKING etc.
IMU: imu driver functions
MAIN: timing
=====================

1. Let's start with "IMU reading -> package into our datastruct" part working.

2. Sensor config:
IMU would have to be sampled above Nyquist rate
gyro reading drifts, so empirically derive offset (read from sensor while perfectly still, repeat many times, average it)
Then we can move onto detection algorithm

3. Feel free to implement all functions in main.
We can always move it out as a separte file later.

4. Some notes on using git:
- use branches temporarily for each feature
- implement small features at a time and push
- commit & merge when the feature is working and ready to be shared

6. PLEASE DO NOT PASTE IN A HUGE SLOP OF AI-GEN CODEDUMP. BITESIZE & VALIDATE THX

**/

int main(void)
{
    while (true)
    {
    }
}