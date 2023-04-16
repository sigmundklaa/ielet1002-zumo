
#ifndef IO_SERIAL_HH__
#define IO_SERIAL_HH__

#include "io.hh"
#include <Arduino.h>
#include <utils/compile.hh>
#include <utils/init.hh>

namespace io
{

template <typename T> class serial_sink : public sink
{
  protected:
    T& hw_serial_;

    size_t
    write_(const uint8_t* data, size_t size) override__
    {
        return hw_serial_.write(data, size);
    }

    size_t
    read_(uint8_t* buf, size_t buf_size) override__
    {
        size_t i = 0;

        for (; hw_serial_.available() && i < buf_size; i++) {
            buf[i] = hw_serial_.read();
        }

        return i;
    }

  public:
    template <typename... Args>
    serial_sink(T& hw_serial, Args... args) : hw_serial_(hw_serial)
    {
        hw_serial_.begin(args...);
    }
};

}; // namespace io

#endif // IO_SERIAL_HH__