#ifndef LSM6DSL_H
#define LSM6DSL_H

#include "mbed.h"
#include "data_struct.h"

class LSM6DSL
{
public:
    // Constants shifted for Mbed 8-bit addressing
    static const uint8_t ADDRESS = (0x6A << 1);

    // Register Map
    enum Register
    {
        WHO_AM_I = 0x0F,
        CTRL1_XL = 0x10,
        CTRL3_C = 0x12,
        OUTZ_L_XL = 0x2C
    };

    LSM6DSL(PinName sda, PinName scl, Timer &t);
    bool init();
    bool readAccel(IMUReading &reading);
    bool enableDataReadyInterrupt();

private:
    I2C _i2c;
    Timer &_timer;
    void writeReg(uint8_t reg, uint8_t val);
    uint8_t readReg(uint8_t reg);
    float _accelSensitivity; // scale (milli-g/LSB)
};

#endif