#include "LSM6DSL.h"

LSM6DSL::LSM6DSL(PinName sda, PinName scl, Timer &t) : _i2c(sda, scl), _timer(t)
{
    _i2c.frequency(400000);
}

void LSM6DSL::writeReg(uint8_t reg, uint8_t val)
{
    char data[2] = {(char)reg, (char)val};
    _i2c.write(ADDRESS, data, 2);
}

uint8_t LSM6DSL::readReg(uint8_t reg)
{
    char r = (char)reg;
    char val = 0;
    _i2c.write(ADDRESS, &r, 1, true);
    _i2c.read(ADDRESS, &val, 1);
    return (uint8_t)val;
}

bool LSM6DSL::init()
{
    if (readReg(WHO_AM_I) != 0x6A)
        return false;
    writeReg(CTRL3_C, 0x44);    // BDU enabled
    writeReg(CTRL1_XL, 0x40);   // 104Hz, 2g(=full range 4g)
    writeReg(CTRL2_G, 0x40); // 104Hz, 245dps(=full range 490deg/sec)
    _accelSensitivity = 0.061f; // (4g range)/(2^16bit values) * 1000milli-g/g
    _gyroSensitivity = 8.75f; // ~ 490dps/(2^16bit values) * 1000milli-deg
    return true;
}

bool LSM6DSL::readIMU(IMUReading &reading)
{
    uint8_t lo, hi;
    float ascale = _accelSensitivity / 1000.0f; // in g
    float gscale = _gyroSensitivity / 1000.0f; // in dps

    // Accel (registers 0x28-0x2D)
    lo = readReg(0x28);
    hi = readReg(0x29);
    reading.ax = (int16_t)((hi << 8) | lo) * ascale;
    lo = readReg(0x2A);
    hi = readReg(0x2B);
    reading.ay = (int16_t)((hi << 8) | lo) * ascale;
    lo = readReg(0x2C);
    hi = readReg(0x2D);
    reading.az = (int16_t)((hi << 8) | lo) * ascale;

    // Gyro (registers 0x22-0x27)
    lo = readReg(0x22); hi = readReg(0x23);
    reading.gx = (int16_t)((hi << 8) | lo) * gscale;
    lo = readReg(0x24); hi = readReg(0x25);
    reading.gy = (int16_t)((hi << 8) | lo) * gscale;
    lo = readReg(0x26); hi = readReg(0x27);
    reading.gz = (int16_t)((hi << 8) | lo) * gscale;

    return true;
}

bool LSM6DSL::enableDataReadyInterrupt()
{
    // 1. Route "Accelerometer Data Ready" to the INT1 pin
    // Register INT1_CTRL (0x0D), Bit 0 is INT1_DRDY_XL
    writeReg(0x0D, 0x01);

    // 2. (Optional) Set interrupt to Latched or Pulsed in CTRL3_C
    // Your previous 0x44 setting already handles basic behavior.
    return true;
}
