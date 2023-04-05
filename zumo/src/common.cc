
#include "common.hh"
#include <io/eeprom.hh>
#include <io/mqtt.hh>
#include <logging/log.hh>
#include <new.h>

#define LOG_MODULE common
LOG_REGISTER(&common::log_sink);

namespace common
{

io::serial_sink log_sink;

static io::mqtt_sink mqtt_sink_(&io::mqtt_client, "/store");

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
