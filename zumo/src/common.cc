
#include "common.hh"
#include <io/eeprom.hh>
#include <io/mqtt.hh>
#include <logging/log.hh>
#include <new.h>
#include <utils/init.hh>

#define LOG_MODULE common
LOG_REGISTER(&common::log_sink);

namespace common
{

io::serial_sink log_sink;

static inline void
init_mqtt_(io::mqtt_sink& sink)
{
    new (&sink) io::mqtt_sink(&io::mqtt_client, "/store/1", "/sync/1");
}

static io::mqtt_sink& mqtt_sink_ = init_guarded(io::mqtt_sink, init_mqtt_);

store<remote_data> remote_store(
    &mqtt_sink_,
    (remote_data){
        .batt_voltage = 0,
    }
);

static io::eeprom_sink eeprom_; /* TODO: implement eeprom */

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
