
#include "battery.hh"
#include <Arduino.h>
#include <io/serial.hh>
#include <logging/log.hh>

static io::serial_sink log_sink_;

#define LOG_MODULE sw_battery
LOG_REGISTER(&log_sink_);

using namespace swbat;

store__::store__(io::sink* sink) : m_sink(sink), balance(0) {}

void
store__::save()
{
    m_packet_buf = (struct packet){
        .voltage = 0,
        .balance = this->balance,
    };

    size_t written = m_sink->write(
        reinterpret_cast<const uint8_t*>(&m_packet_buf), sizeof(m_packet_buf)
    );

    if (written != sizeof(m_packet_buf)) {
        LOG_ERR(
            << "store: unable to save (only " << String(written)
            << " bytes written)\n"
        );
    }
}
