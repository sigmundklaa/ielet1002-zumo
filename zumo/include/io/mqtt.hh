
#ifndef IO_MQTT_HH__
#define IO_MQTT_HH__

#include "io.hh"
#include <PubSubClient.h>
#include <assert.h>
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
    PubSubClient& m_client;
    const char* m_pub_topic;

  public:
    mqtt_sink(PubSubClient& client, const char* topic)
        : m_client(client), m_pub_topic(topic)
    {
    }

    size_t
    write_(const uint8_t* data, size_t size) override__
    {
        return m_client.publish(m_pub_topic, data, size) ? size : 0;
    }

    size_t
    read_(uint8_t* buf, size_t buf_size) override__
    {
        assert(0);
        return 0;
    }
};

}; // namespace io

#endif // IO_MQTT_HH__