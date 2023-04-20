#pragma once
#include <stdint.h>

class Zumo32U4IMU
{
  public:
    int16_t a[3];

    int
    init()
    {
        return 1;
    }

    void
    enableDefault()
    {
    }

    void
    readAcc()
    {
    }
};

class Zumo32U4Encoders
{
  public:
    int16_t
    getCountsLeft()
    {
        return 0;
    }

    int16_t
    getCountsRight()
    {
        return 0;
    }
};

class Zumo32U4Motors
{
  public:
    static void
    setLeftSpeed(int16_t s)
    {
    }

    static void
    setRightSpeed(int16_t s)
    {
    }
};