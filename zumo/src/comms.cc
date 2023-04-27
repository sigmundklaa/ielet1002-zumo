
#include "comms.hh"
#include "common.hh"
#include <io/redirect.hh>
#include <logging/log.hh>

#define LOG_MODULE comms
LOG_REGISTER(common::log_gateway);

namespace rd = io::redirect;

namespace comms
{

uint8_t buf_[256];

static rd::redirect_gateway*
find_gateway_(rd::node_type node)
{
    // clang-format off
    switch (node) {
    #define X(name, node) case node: return &name;
        COMMS_NODES__
    #undef X
    }
    // clang-format on

    return nullptr;
}

static void
handle_input_(uint8_t* data, size_t size)
{
    io::redirect::header* header = (io::redirect::header*)data;
    io::redirect::redirect_gateway* gw = find_gateway_(header->node);

    if (gw == nullptr) {
        LOG_ERR(<< "gateway is null");
        return;
    }

    gw->push(data + sizeof(*header), header->size);
}

void
on_tick()
{
    static uint64_t last_read_us;
    uint64_t tmp = micros();

    /* A delay is required to allow for all the bytes to be filled in to the
     * buffer */
    if (tmp - last_read_us <= 0) {
        return;
    }
    last_read_us = tmp;

    size_t bread = common::serial_gateway_.read(buf_, sizeof(buf_));
    if (bread == 0) {
        return;
    }

    io::redirect::header* header = (io::redirect::header*)buf_;
    LOG_INFO(
        << "recieved over serial: " << String(bread) << "bytes; node "
        << String(header->node) << ", data size " << String(header->size)
    );
    bread += common::serial_gateway_.read(buf_ + sizeof(*header), header->size);

    handle_input_(buf_, bread);
}

}; // namespace comms