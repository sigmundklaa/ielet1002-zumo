
#pragma once

#if !defined(__unix__)
#include_next <Arduino.h>
#else

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef char boolean;
#define true (1)
#define false (0)

class String
{
  public:
    const char*
    c_str() const
    {
        return reinterpret_cast<const char*>(0);
    }

    size_t
    length() const
    {
        return 0;
    }
};

inline uint64_t
micros()
{
    time_t t = time(NULL);

    return t * 1e6;
}

inline long
map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

class Serial_
{
  public:
    int
    available()
    {
        return 1;
    }

    void
    begin(int baud)
    {
    }

    size_t
    read()
    {
        return 0;
    }

    size_t
    write(const uint8_t* buf, size_t size)
    {
        return size;
    }
};

static Serial_ Serial;

#endif