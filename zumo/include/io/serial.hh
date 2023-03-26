
#ifndef IO_SERIAL_HH__
#define IO_SERIAL_HH__

#include "io.hh"
#include <Arduino.h>

namespace io
{

class serial_sink : public sink
{
  protected:
    size_t
    write_(const uint8_t* data, size_t size) override
    {
        return Serial.write(data, size);
    }

    size_t
    read_(uint8_t* buf, size_t buf_size) override
    {
        size_t i = 0;

        for (; Serial.available() && i < buf_size; i++) {
            buf[i] = Serial.read();
        }

        return i;
    }
};

}; // namespace io

#endif // IO_SERIAL_HH__