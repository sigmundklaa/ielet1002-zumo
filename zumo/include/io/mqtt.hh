
#ifndef IO_MQTT_HH__
#define IO_MQTT_HH__

#include "io.hh"
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

enum mqtt_node_type {
    ZUMO = 0x1,
};

#define IO_MQTT_TRIPLES__(x, y, z) x##y##z
#define IO_MQTT_TRIPLES_(x, y, z) IO_MQTT_TRIPLES__(x, y, z)

extern PubSubClient& mqtt_client;

/**
 * @brief Helper class that ensures the MQTT connection is initialized when the
 * header is included, so that any dependency on the mqtt_client works as
 * expected. This is because we need to establish a connection and request data
 * before any @code mqtt_sink @endcode  using the client can be constructed.
 *
 */
static class mqtt_initializer_
{
  public:
    mqtt_initializer_();
} IO_MQTT_TRIPLES_(mqtt_init_, __LINE__, __);

}; // namespace io

#endif // IO_MQTT_HH__