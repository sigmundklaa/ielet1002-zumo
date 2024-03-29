
#ifndef IO_REDIRECT_HH__
#define IO_REDIRECT_HH__

#include <assert.h>
#include <io/io.hh>

namespace io
{
namespace redirect
{

enum node_type {
    NODE_MQTT_REPORT_1,
    NODE_MQTT_STORE_1,
    NODE_MQTT_CHARGE_1,
    NODE_MQTT_INIT_1,
    NODE_MQTT_CONTROL_1,
    NODE_MQTT_TRASH_1,
} __attribute__((packed));

/* Ensure that the enum is the expected size. */
static_assert(sizeof(node_type) == 1, "size of header type is 1");

struct __attribute__((packed)) header {
    node_type node;
    uint8_t size;
};

#define MAX_PACKET_SIZE_ (256 - sizeof(header))

/**
 * @brief Prepare header for redirecting.
 *
 * @param type
 * @param buf Temporary buffer to write the header to
 * @param size Size of payload
 * @return size_t
 */
inline size_t
prepare_header(node_type type, uint8_t* buf, size_t size)
{
    assert(size <= UINT8_MAX);

    header* h = reinterpret_cast<header*>(buf);
    h->node = type;
    h->size = static_cast<uint8_t>(size);

    if (size > MAX_PACKET_SIZE_) {
        return 0;
    }

    return sizeof(*h);
}

/**
 * @brief Get the total size of a packet, header size inclusive
 *
 * @param data_size
 * @return size_t
 */
inline size_t
total_size(size_t data_size)
{
    return sizeof(header) + data_size;
}

/**
 * @brief Gateway that is used for redirecting data over a connected router and
 * to the endpoint determined by the node type.
 *
 */
class redirect_gateway : public pushable_gateway
{
  protected:
    gateway& parent_;
    node_type type_;
    uint8_t* buf_;

    uint8_t*
    prepare_buf_(size_t size)
    {
        size_t header_sz = prepare_header(type_, buf_, size);

        return buf_ + header_sz;
    }

    size_t
    write_(const uint8_t* data, size_t size)
    {
        uint8_t* data_buf = prepare_buf_(size);
        ::memcpy(data_buf, data, size);

        return parent_.write(buf_, total_size(size));
    }

  public:
    redirect_gateway(gateway& parent, node_type redirect_type, uint8_t* buf)
        : parent_(parent), type_(redirect_type), buf_(buf)
    {
    }
};
}; // namespace redirect

}; // namespace io

#endif // IO_REDIRECT_HH__