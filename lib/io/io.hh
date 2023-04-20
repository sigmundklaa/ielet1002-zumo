
#ifndef IO_IO_H__
#define IO_IO_H__

#include <mem/arena.hh>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utils/compile.hh>
#include <utils/init.hh>

namespace io
{

#define IO_MAX_PRINT_SIZE_ (512)

static mem::arena<256>& read_buf =
    init_guarded(mem::arena<256>, utils::init_empty);

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
    template <typename T>
    size_t
    write(const T* data, size_t size)
    {
        return this->write_(reinterpret_cast<const uint8_t*>(data), size);
    }

    template <typename T>
    size_t
    read(T* buf, size_t buf_size)
    {
        return this->read_(reinterpret_cast<uint8_t*>(buf), buf_size);
    }
};

/**
 * @brief The pushable sink is used for when the implementation of the
 * underlying device uses callbacks. We want to be able to poll the sink through
 * the read method, and so we push the data to the sink from the callback
 * function and temporarily store it until we call read.
 *
 */
class pushable_sink : public sink
{
  protected:
    uint8_t* read_data_;
    size_t read_size_;

    virtual size_t write_(const uint8_t* data, size_t size);

    size_t
    read_(uint8_t* buf, size_t buf_size) override__
    {
        if (!read_size_) {
            return 0;
        }

        buf_size = buf_size < read_size_ ? buf_size : read_size_;
        read_size_ -= buf_size;

        memcpy(buf, read_data_, buf_size);

        if (!read_size_) {
            io::read_buf.dealloc(read_data_);
        }

        return buf_size;
    }

  public:
    void
    push(const uint8_t* data, size_t size)
    {
        /* Data has not been read yet. */
        if (read_size_ != 0) {
            return;
        }

        read_data_ = (uint8_t*)io::read_buf.alloc(size);

        /* Unable to allocate space in the buffer; forced to discard */
        if (read_data_ == nullptr) {
            return;
        }

        read_size_ = size;

        ::memcpy(read_data_, data, read_size_);
    }
};

}; // namespace io

#endif // IO_IO_H__