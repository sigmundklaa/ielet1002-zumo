
#ifndef IO_MQTT_HH__
#define IO_MQTT_HH__

#include "io.hh"
#include <Client.h>
#include <IPAddress.h>
#include <PubSubClient.h>
#include <assert.h>
#include <stddef.h>
#include <utils/compile.hh>
#include <utils/init.hh>
#include <utils/macros.hh>
#include <wifi.hh>

namespace io
{

static inline void
mqtt_client_init(PubSubClient& psc)
{
    static WiFiClient wific;

    new (&psc) PubSubClient(wific);
}

/*
Ensure initialization of the PubSubClient. If we didn't do this the client could
be used by another constructor before it is initialized, and considering the
PubSubClient uses malloc to allocate its buffer we could get a segmentation
fault when trying to e.g. subscribe to a topic from within a constructor.
*/
static PubSubClient& mqtt_client = init_guarded(PubSubClient, mqtt_client_init);

/**
 * @brief MQTT gateway that communicates with the MQTT broker. Each instance
 * only communicates with one topic
 *
 */
class mqtt_gateway : public pushable_gateway
{
  protected:
    PubSubClient* m_client;
    const char* m_pub_topic;

    size_t
    write_(const uint8_t* data, size_t size) override__
    {
        return m_client->publish(m_pub_topic, data, size) ? size : 0;
    }

  public:
    mqtt_gateway(
        PubSubClient* client, const char* pub_topic, const char* sub_topic
    );

    uint8_t
    avail() const
    {
        return read_size_ > 0;
    }

    PubSubClient*
    ps_client() const
    {
        return m_client;
    }
};

#if 0
#define IO_MQTT_NUM_gatewayS_MAX_ (10)

class mqtt_handler__
{
  protected:
    static void callback_(char* topic, uint8_t* payload, unsigned int size);

    struct entry_ {
        const char* topic;
        mqtt_gateway* gateway;
    };

    entry_ entries_[IO_MQTT_NUM_gatewayS_MAX_];
    size_t n_entries_;

  public:
    void
    register_gateway(const char* topic, mqtt_gateway* gateway)
    {
        if (n_entries_ >= IO_MQTT_NUM_gatewayS_MAX_) {
            /* TODO: error handle */
            return;
        }

        entries_[n_entries_++] = (entry_){
            .topic = topic,
            .gateway = gateway,
        };

        mqtt_client.subscribe(topic);
    }

    void
    handle_callback(char* topic, uint8_t* payload, size_t size)
    {
        for (size_t i = 0; i < n_entries_; i++) {
            entry_* e = &entries_[i];

            if (strcmp(e->topic, topic) == 0) {
                e->gateway->push(payload, size);
                return;
            }
        }

        /* TODO: error handle */
    }
};

static mqtt_handler__& mqtt_handler =
    init_guarded(mqtt_handler__, utils::init_empty);

inline mqtt_gateway::mqtt_gateway(
    PubSubClient* client, const char* pub_topic, const char* sub_topic
)
    : m_client(client), m_pub_topic(pub_topic)
{
    if (sub_topic != nullptr) {
        mqtt_handler.register_gateway(sub_topic, this);
    }
}

inline void
mqtt_handler__::callback_(char* topic, uint8_t* payload, unsigned int size)
{
    mqtt_handler.handle_callback(topic, payload, static_cast<size_t>(size));
}

#define IO_MQTT_NODE_ZUMO 1

#define IO_MQTT_PATH(path, node) UTILS_STR_CONCAT(path, UTILS_STR(node))
#else
inline mqtt_gateway::mqtt_gateway(
    PubSubClient* client, const char* pub_topic, const char* sub_topic
)
    : m_client(client), m_pub_topic(pub_topic)
{
}
#endif

}; // namespace io

#endif // IO_MQTT_HH__
