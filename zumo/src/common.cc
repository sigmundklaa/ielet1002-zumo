
#include "common.hh"
#include <Arduino.h>
#include <io/eeprom.hh>
#include <io/mqtt.hh>
#include <logging/log.hh>
#include <utils/init.hh>
#include <utils/new.hh>

#define LOG_MODULE common
LOG_REGISTER(&common::log_sink);

#define SYNC_TIMEOUT_US_ (5e6)

namespace common
{

io::serial_sink log_sink;

/* TODO: sinks should not use an init guard as there may be multiple instances.
 * This can probably be implemented with an init guard for the store instead. */
static inline void
init_mqtt_(io::mqtt_sink& sink)
{
    new (&sink) io::mqtt_sink(
        &io::mqtt_client, IO_MQTT_PATH("/store/", IO_MQTT_NODE_ZUMO),
        IO_MQTT_PATH("/sync/", IO_MQTT_NODE_ZUMO)
    );

    LOG_INFO(<< "requesting sync from remote");

    /* Send no data to indicate we are requesting sync data */
    sink.write("", 0);

    /* Wait to recieve data before we can continue. Only try for X amount of
     * microseconds to prevent hang */
    for (uint64_t start = micros(); micros() - start < SYNC_TIMEOUT_US_;) {
        if (sink.avail()) {
            return;
        }

        sink.ps_client()->loop();
    }

    LOG_ERR(<< "error syncing data");
}

static io::mqtt_sink& mqtt_sink_ = init_guarded(io::mqtt_sink, init_mqtt_);

store<remote_data> remote_store(
    &mqtt_sink_,
    (remote_data){
        .batt_voltage = 0,
    }
);

static io::eeprom_sink eeprom_;

store<local_data> local_store(
    &eeprom_,
    (local_data){

    }
);

}; // namespace common

void
io::mqtt_client_init(PubSubClient& client)
{
    new (&client) PubSubClient();
}
