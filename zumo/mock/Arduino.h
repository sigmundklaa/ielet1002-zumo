
#pragma once

#if !defined(__unix__)
#include_next <Arduino.h>
#else

#include <stdint.h>
#include <stdlib.h>

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
    return 0;
}

#endif