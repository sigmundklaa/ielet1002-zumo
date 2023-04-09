
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

namespace io
{

/**
 * @brief Defined by the user.
 *
 */
extern void mqtt_client_init(PubSubClient&);

/*
Ensure initialization of the PubSubClient. If we didn't do this the client could
be used by another constructor before it is initialized, and considering the
PubSubClient uses malloc to allocate its buffer we could get a segmentation
fault when trying to e.g. subscribe to a topic from within a constructor.
*/
static PubSubClient& mqtt_client = init_guarded(PubSubClient, mqtt_client_init);

/**
 * @brief MQTT sink that communicates with the MQTT broker. Each instance only
 * communicates with one topic
 *
 */
class mqtt_sink : public sink
{
  protected:
    struct {
        uint8_t mem[128];
        size_t index;
    } buf_;

    PubSubClient* m_client;
    const char* m_pub_topic;

    size_t
    write_(const uint8_t* data, size_t size) override__
    {
        return m_client->publish(m_pub_topic, data, size) ? size : 0;
    }

    size_t
    read_(uint8_t* buf, size_t buf_size) override__
    {
        if (!buf_.index) {
            return 0;
        }

        buf_size = buf_size < buf_.index ? buf_size : buf_.index;

        memcpy(buf, buf_.mem, buf_size);
        return buf_size;
    }

  public:
    mqtt_sink(
        PubSubClient* client, const char* pub_topic, const char* sub_topic
    );

    void
    push(const uint8_t* data, size_t size)
    {
        if (size > sizeof(buf_.mem)) {
            /* TODO: err handle */
            return;
        }

        if (buf_.index + size >= sizeof(buf_.mem)) {
            buf_.index = 0;
        }

        memcpy(buf_.mem + buf_.index, data, size);
        buf_.index += size;
    }

    uint8_t
    avail() const
    {
        return buf_.index > 0;
    }

    PubSubClient*
    ps_client() const
    {
        return m_client;
    }
};

#define IO_MQTT_NUM_SINKS_MAX_ (10)

class mqtt_handler__
{
  protected:
    static void callback_(char* topic, uint8_t* payload, unsigned int size);

    struct entry_ {
        const char* topic;
        mqtt_sink* sink;
    };

    entry_ entries_[IO_MQTT_NUM_SINKS_MAX_];
    size_t n_entries_;

  public:
    void
    register_sink(const char* topic, mqtt_sink* sink)
    {
        if (n_entries_ >= IO_MQTT_NUM_SINKS_MAX_) {
            /* TODO: error handle */
            return;
        }

        entries_[n_entries_++] = (entry_){
            .topic = topic,
            .sink = sink,
        };

        mqtt_client.subscribe(topic);
    }

    void
    handle_callback(char* topic, uint8_t* payload, size_t size)
    {
        for (size_t i = 0; i < n_entries_; i++) {
            entry_* e = &entries_[i];

            if (strcmp(e->topic, topic) == 0) {
                e->sink->push(payload, size);
                return;
            }
        }

        /* TODO: error handle */
    }
};

static mqtt_handler__& mqtt_handler =
    init_guarded(mqtt_handler__, utils::init_empty);

inline mqtt_sink::mqtt_sink(
    PubSubClient* client, const char* pub_topic, const char* sub_topic
)
    : m_client(client), m_pub_topic(pub_topic)
{
    if (sub_topic != nullptr) {
        mqtt_handler.register_sink(sub_topic, this);
    }
}

inline void
mqtt_handler__::callback_(char* topic, uint8_t* payload, unsigned int size)
{
    mqtt_handler.handle_callback(topic, payload, static_cast<size_t>(size));
}

#define IO_MQTT_NODE_ZUMO 1

#define IO_MQTT_PATH(path, node) UTILS_STR_CONCAT(path, UTILS_STR(node))

}; // namespace io

#endif // IO_MQTT_HH__