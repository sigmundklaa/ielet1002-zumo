
#ifndef IO_MQTT_HH__
#define IO_MQTT_HH__

#include "io.hh"
#include <PubSubClient.h>
#include <logging/log.hh>

#define LOG_MODULE io_mqtt

#if LOG_LEVEL <= LOG_LEVEL_DISABLED
#include "serial.hh"

static io::serial_sink sink_;
LOG_REGISTER(reinterpret_cast<io::sink*>(&sink_));
#endif

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
    write_(const uint8_t* data, size_t size) override
    {
        return m_client.publish(m_pub_topic, data, size) ? size : 0;
    }

    size_t
    read_(uint8_t* buf, size_t buf_size) override
    {
        // TOOD: assert
        LOG_ERR(<< "mqtt_sink does not support reading\n");
        return 0;
    }
};

}; // namespace io

#endif // IO_MQTT_HH__