#include "mbed.h"
#include <chrono> // c++ time library
#include "LSM6DSL.h"
#include "gesture.h"
#include "states.h"
using namespace std::chrono;

/**
=== File structure===
GESTURE: vectors and math
STATES: IDLE, RECORDING, UNLOCKING etc.
IMU: imu driver functions
MAIN: timing
=====================
*/

LSM6DSL imu(PB_11, PB_10); // IMU: I2C config
// Teleplot
BufferedSerial serial_port(USBTX, USBRX, 115200);
FileHandle *mbed::mbed_override_console(int)
{
    return &serial_port;
}
InterruptIn imu_int1(PB_2); // IMU: interrupt config
Timer t;

int main()
{
    if (!imu.init())
    {
        printf("Sensor Error!\r\n");
        return -1;
    }

    t.start();
    uint32_t lastPrintTime = 0;

    while (true)
    {
        float x, y, z;
        // Read IMU
        if (imu.readAccelXYZ(x, y, z))
        {
            // gesture detection
        }

        // Serial monitor
        uint32_t currentTime = duration_cast<milliseconds>(t.elapsed_time()).count();
        if (currentTime - lastPrintTime >= 100)
        {
            lastPrintTime = currentTime;
            printf("X: %.3f | Y: %.3f | Z: %.3f\r\n", x, y, z);
        }
        ThisThread::sleep_for(2ms);
    }
}
