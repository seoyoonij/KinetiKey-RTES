#include "mbed.h"
#include <chrono> // c++ time library
#include "LSM6DSL.h"
#include "data_struct.h"
#include "gesture.h"
#include "states.h"
using namespace std::chrono;

Timer t;                      // clock
LSM6DSL imu(PB_11, PB_10, t); // IMU: I2C config
// InterruptIn imu_int1(PB_2);   // IMU: interrupt config

// Teleplot config
BufferedSerial serial_port(USBTX, USBRX, 115200);
FileHandle *mbed::mbed_override_console(int)
{
    return &serial_port;
}

int main()
{
    t.start();
    if (!imu.init())
    {
        printf("IMU init failed\r\n");
        return -1;
    }

    IMUReading currentReading;

    while (true)
    {
        float x, y, z;
        // Read IMU
        if (imu.readAccel(currentReading))
        {
            // gesture detection logic

            // Serial monitor
            printf("Time: %lu ms | X: %.2f g | Y: %.2f g | Z: %.2f g\r\n", currentReading.timestamp_ms, currentReading.x, currentReading.y, currentReading.z);
        }

        ThisThread::sleep_for(5ms);
    }
}
