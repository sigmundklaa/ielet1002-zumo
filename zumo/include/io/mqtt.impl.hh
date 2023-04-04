/* TODO: this should be moved to a .cc file */

#ifndef IO_MQTT_IMPL_HH__
#define IO_MQTT_IMPL_HH__

#include "mqtt.hh"
#include <string.h>

namespace io
{
namespace mqtt_impl__
{

static int nifty_;

/* Allocating a buffer the size of the client, instead of the client itself
 * allows us to bypass the automatic call to the constructor. The client will
 * then be manually constructed in @code
 * io::mqtt_initializer_::mqtt_initializer_() @endcode  */
static uint8_t mem_[sizeof(PubSubClient)];

static void
mqtt_callback_tmp_(char* topic, uint8_t* payload, unsigned int size)
{
    if (strcmp(topic, "/init") != 0) {
        return;
    }

    if (*payload != mqtt_node_type::ZUMO) {
        return;
    }
}

}; // namespace mqtt_impl__

PubSubClient& mqtt_client = reinterpret_cast<PubSubClient&>(mqtt_impl__::mem_);

mqtt_initializer_::mqtt_initializer_()
{
    /* Only initialize once */
    if (mqtt_impl__::nifty_++ != 0) {
        return;
    }

    new (&mqtt_client) PubSubClient();

    mqtt_client.setCallback(mqtt_impl__::mqtt_callback_tmp_);
}
}; // namespace io

#endif // IO_MQTT_IMPL_HH__