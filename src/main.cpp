#include "mbed.h"
#include "LSM6DSL.h"
#include "gesture.h"
#include "states.h"

/**
TODO:

=== File structure===
GESTURE: vectors and math
STATES: IDLE, RECORDING, UNLOCKING etc.
IMU: imu driver functions
MAIN: timing
=====================
*/

LSM6DSL imu(PB_11, PB_10); // IMU: I2C config
InterruptIn imu_int1(PB_2); // IMU: interrupt config
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;

int main()
{
    if (!imu.init())
    {
        printf("Sensor Error!\n");
        return -1;
    }

    while (true)
    {
        float acc_z = imu.readAccelZ();

        // ... (Do your FFT and Moving Average math here)

        printf(">Raw_Acc:%.2f\n", acc_z);
        ThisThread::sleep_for(9ms);
    }
}