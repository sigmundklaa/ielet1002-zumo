
#ifndef BATTERY_HH__
#define BATTERY_HH__

#include <io/mqtt.hh>
#include <logging/log.hh>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace swbat
{

/**
 * @brief Contain variables that are reliant on an external storage device (e.g.
 * cloud, nonvolatile physical storage etc.).
 *
 */
template <class T> class store__
{
  protected:
    /**
     * @brief Only used when sending and recieving data. This is here because we
     * don't want to work with the inconvience of packed structs when we don't
     * have to. For example, taking the memory address of a member of a packed
     * struct and then dereferencing that could cause a runtime error, as the
     * address may not be aligned.
     *
     */
    struct __attribute__((packed)) packet {
        uint8_t voltage;
        uint32_t balance;
    };
    packet m_packet_buf;

    const io::sink* m_sink;

  public:
    uint32_t balance;

    store__(const T* sink) : balance(0), m_sink(sink) {}

    void
    save()
    {
        m_packet_buf = (struct packet){
            .balance = this->balance,
        };

        size_t written = m_sink->write(&m_packet_buf, sizeof(m_packet_buf));

        if (written != sizeof(m_packet_buf)) {
            LOG_ERR(
                << "store: unable to save (only " << String(written)
                << " bytes written)\n"
            );
        }
    }
};
extern store__<io::mqtt_sink> store_mqtt;

class battery__
{
  protected:
  public:
    battery__() = default;
};
extern battery__ battery;

}; // namespace swbat

#endif // BATTERY_H__