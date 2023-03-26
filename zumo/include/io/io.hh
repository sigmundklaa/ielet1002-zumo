
#ifndef IO_IO_H__
#define IO_IO_H__

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace io
{

/**
 * @brief Pure virtual class for the sink interface
 *
 */
class sink
{
  protected:
    virtual size_t write_(const uint8_t* data, size_t size) = 0;
    virtual size_t read_(uint8_t* buf, size_t size) = 0;

  public:
    size_t
    write(const uint8_t* data, size_t size)
    {
        return this->write_(data, size);
    }

    size_t
    read(uint8_t* buf, size_t buf_size)
    {
        return this->read_(buf, buf_size);
    }
};

}; // namespace io

#endif // IO_IO_H__