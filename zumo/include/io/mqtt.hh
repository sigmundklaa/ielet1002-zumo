
#ifndef IO_MQTT_HH__
#define IO_MQTT_HH__

#include "io.hh"
#include <Client.h>
#include <IPAddress.h>
#include <PubSubClient.h>
#include <assert.h>
#include <new.h>
#include <stddef.h>
#include <utils/compile.hh>

namespace io
{

/**
 * @brief MQTT sink that communicates with the MQTT broker. Each instance only
 * communicates with one topic
 *
 */
class mqtt_sink : public sink
{
  protected:
    PubSubClient* m_client;
    const char* m_pub_topic;

  public:
    mqtt_sink(PubSubClient* client, const char* topic)
        : m_client(client), m_pub_topic(topic)
    {
    }

    size_t
    write_(const uint8_t* data, size_t size) override__
    {
        return m_client->publish(m_pub_topic, data, size) ? size : 0;
    }

    size_t
    read_(uint8_t* buf, size_t buf_size) override__
    {
        assert(0);
        return 0;
    }
};

#define IO_MQTT_NODE_ZUMO (1)

#ifndef IO_MQTT_NODE
#define IO_MQTT_NODE IO_MQTT_NODE_ZUMO
#endif

#define IO_MQTT_TRIPLES__(x, y, z) x##y##z
#define IO_MQTT_TRIPLES_(x, y, z) IO_MQTT_TRIPLES__(x, y, z)

extern PubSubClient& mqtt_client;

/**
 * @brief Helper class that ensures the MQTT connection is initialized when the
 * an object of this type is declared, so that any dependency on the mqtt_client
 * works as expected. This is because we need to establish a connection and
 * request data before any @code mqtt_sink @endcode  using the client can be
 * constructed. NOTE: This initializer assumes that there is only one MQTT
 * connection active on this device.
 *
 * Example:
 * static mqtt_init_guard guard_(IO_MQTT_NODE_ZUMO);
 * // After this point the mqtt client is guaranteed to be properly initialized
 */
class mqtt_init_guard
{
  public:
    mqtt_init_guard(
        uint8_t node, const IPAddress& addr, uint16_t port, Client& client
    );
};

}; // namespace io

#endif // IO_MQTT_HH__