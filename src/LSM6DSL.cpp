#include "LSM6DSL.h"

LSM6DSL::LSM6DSL(PinName sda, PinName scl) : _i2c(sda, scl)
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
    _accelSensitivity = 0.061f; // (4g range)/(2^16bit values) * 1000milli-g/g
    return true;
}

bool LSM6DSL::readAccelXYZ(float &x, float &y, float &z)
{
    uint8_t lo, hi;
    float scale = _accelSensitivity / 1000.0f; // in g

    lo = readReg(0x28);
    hi = readReg(0x29);
    x = (int16_t)((hi << 8) | lo) * scale;

    lo = readReg(0x2A);
    hi = readReg(0x2B);
    y = (int16_t)((hi << 8) | lo) * scale;

    lo = readReg(0x2C);
    hi = readReg(0x2D);
    z = (int16_t)((hi << 8) | lo) * scale;

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
