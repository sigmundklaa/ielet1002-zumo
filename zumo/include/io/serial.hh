
#ifndef IO_SERIAL_HH__
#define IO_SERIAL_HH__

#if defined(__unix__)
#include "std.hh"

namespace io
{
typedef std_sink serial_sink;
};

#else

#include "io.hh"
#include <Arduino.h>
#include <utils/compile.hh>

namespace io
{

class serial_sink : public sink
{
  protected:
    size_t
    write_(const uint8_t* data, size_t size) override__
    {
        return Serial.write(data, size);
    }

    size_t
    read_(uint8_t* buf, size_t buf_size) override__
    {
        size_t i = 0;

        for (; Serial.available() && i < buf_size; i++) {
            buf[i] = Serial.read();
        }

        return i;
    }
};

}; // namespace io

#endif

#endif // IO_SERIAL_HH__