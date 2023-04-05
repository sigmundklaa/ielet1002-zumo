/* TODO: this should be moved to a .cc file */

#ifndef IO_MQTT_IMPL_HH__
#define IO_MQTT_IMPL_HH__

#include "mqtt.hh"
#include <string.h>
#include <utils/mem.hh>

namespace io
{

/* This namespace is only necessary as long as this is not kept in a .cc file */
namespace mqtt_impl__
{

static int nifty_;

/* Allocating a buffer the size of the client, instead of the client itself
 * allows us to bypass the automatic call to the constructor. The client will
 * then be manually constructed in @code
 * io::mqtt_initializer_::mqtt_initializer_() @endcode  */
static uint8_t mem_[aligned__(sizeof(PubSubClient), alignof__(PubSubClient))];

}; // namespace mqtt_impl__

PubSubClient& mqtt_client = reinterpret_cast<PubSubClient&>(mqtt_impl__::mem_);

#define CONNECT_TIMEOUT_US_ (5e6) /* 5 seconds */

mqtt_init_guard::mqtt_init_guard(
    const IPAddress& addr, uint16_t port, Client& client
)
{
    init_((mqtt_init_guard::params){
        .addr = addr,
        .port = port,
        .client = client,
    });
}

mqtt_init_guard::mqtt_init_guard(const params& args) { init_(args); }

void
mqtt_init_guard::init_(const params& args)
{
    /* Only initialize once */
    if (mqtt_impl__::nifty_++ != 0) {
        return;
    }

    new (&mqtt_client) PubSubClient(args.addr, args.port, args.client);
}

}; // namespace io

#endif // IO_MQTT_IMPL_HH__