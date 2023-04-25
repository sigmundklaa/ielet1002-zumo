
#ifndef IO_REDIRECT_HH__
#define IO_REDIRECT_HH__

#include <assert.h>
#include <io/io.hh>

namespace io
{
namespace redirect
{

enum header_type {
    PACKET_MQTT,
    PACKET_ESPNOW,
    PACKET_HOST,
} __attribute__((packed));

static_assert(sizeof(header_type) == 1, "size of header type is 1");

struct __attribute__((packed)) header {
    header_type type;
    uint8_t dst_size;
    uint8_t size;
};

#define MAX_PACKET_SIZE_ (256 - sizeof(header))

inline size_t
prepare_header(
    header_type type, uint8_t* buf, const void* dst_data, size_t dst_size,
    size_t data_size
)
{
    size_t size = dst_size + data_size;
    assert(size <= UINT8_MAX);

    header* h = reinterpret_cast<header*>(buf);
    h->type = type;
    h->dst_size = static_cast<uint8_t>(dst_size);
    h->size = data_size;

    if (size > MAX_PACKET_SIZE_) {
        return 0;
    }

    if (dst_size > 0) {
        ::memcpy(buf + sizeof(*h), dst_data, dst_size);
    }

    return sizeof(*h) + dst_size;
}

inline size_t
calc_size(size_t size, size_t dst_size)
{
    return sizeof(header) + dst_size + size;
}

class redirect_gateway : public pushable_gateway
{
  protected:
    static uint8_t buf_[MAX_PACKET_SIZE_];

    gateway& parent_;
    header_type type_;

    const uint8_t* dst_;
    size_t dst_size_;

    uint8_t*
    prepare_buf_(size_t size)
    {
        header h = {
            .type = type_,
            .dst_size = static_cast<uint8_t>(dst_size_),
            .size = static_cast<uint8_t>(size),
        };

        ::memcpy(redirect_gateway::buf_, &h, sizeof(h));
        ::memcpy(redirect_gateway::buf_ + sizeof(h), dst_, dst_size_);

        return redirect_gateway::buf_ + sizeof(h) + dst_size_;
    }

    size_t
    write_(const uint8_t* data, size_t size)
    {
        uint8_t* buf = prepare_buf_(size);
        ::memcpy(buf, data, size);

        return parent_.write(buf, calc_size(size, dst_size_));
    }

  public:
    redirect_gateway(
        gateway& parent, header_type redirect_type, const uint8_t* dst,
        size_t dst_size
    )
        : parent_(parent), type_(redirect_type), dst_(dst), dst_size_(dst_size)
    {
    }
};
inline uint8_t redirect_gateway::buf_[MAX_PACKET_SIZE_];
}; // namespace redirect

}; // namespace io

#endif // IO_REDIRECT_HH__