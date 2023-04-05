/* TODO: this should be moved to a .cc file */

#ifndef IO_MQTT_IMPL_HH__
#define IO_MQTT_IMPL_HH__

#include "mqtt.hh"
#include <string.h>
#include <utils/mem.hh>

namespace io
{

#define ENDPOINT_SYNC_ "/sync"
#define ENDPOINT_INIT_ "/init"

/* This namespace is only necessary as long as this is not kept in a .cc file */
namespace mqtt_impl__
{

static int inited_;
static int nifty_;
static uint8_t node_;

/* Allocating a buffer the size of the client, instead of the client itself
 * allows us to bypass the automatic call to the constructor. The client will
 * then be manually constructed in @code
 * io::mqtt_initializer_::mqtt_initializer_() @endcode  */
static uint8_t mem_[aligned__(sizeof(PubSubClient), alignof__(PubSubClient))];

static void
mqtt_callback_tmp_(char* topic, uint8_t* payload, unsigned int size)
{
    if (strcmp(topic, ENDPOINT_SYNC_) != 0) {
        return;
    }

    if (*payload != node_) {
        return;
    }

    io::mqtt_client.unsubscribe(ENDPOINT_SYNC_);

    inited_ = 1;
}

}; // namespace mqtt_impl__

PubSubClient& mqtt_client = reinterpret_cast<PubSubClient&>(mqtt_impl__::mem_);

#define CONNECT_TIMEOUT_US_ (5e6) /* 5 seconds */

mqtt_init_guard::mqtt_init_guard(
    uint8_t node, const IPAddress& addr, uint16_t port, Client& client
)
{
    /* Only initialize once */
    if (mqtt_impl__::nifty_++ != 0) {
        return;
    }

    /* When multiple devices boot up around the same time, an init  */
    mqtt_impl__::node_ = node;

    new (&mqtt_client) PubSubClient(addr, port, client);

    /* Publish an initialization request and wait for the result.  */
    mqtt_client.subscribe(ENDPOINT_SYNC_);
    mqtt_client.publish(
        ENDPOINT_INIT_, &mqtt_impl__::node_, sizeof(mqtt_impl__::node_)
    );

    mqtt_client.setCallback(mqtt_impl__::mqtt_callback_tmp_);

    for (uint64_t start = micros(); micros() - start < CONNECT_TIMEOUT_US_;) {
        if (mqtt_impl__::inited_) {
            return;
        }

        mqtt_client.loop();
    }

    /* TODO: error handle */
}
}; // namespace io

#endif // IO_MQTT_IMPL_HH__