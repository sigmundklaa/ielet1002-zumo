
#include "common.hh"
#include <Arduino.h>
#include <io/eeprom.hh>
#include <io/mqtt.hh>
#include <logging/log.hh>
#include <utils/init.hh>
#include <utils/new.hh>

#define LOG_MODULE common
LOG_REGISTER(common::log_gateway);

#define SYNC_TIMEOUT_US_ (5e6)

namespace common
{

static io::mqtt_gateway mqtt_gateway_(
    &io::mqtt_client, IO_MQTT_PATH("/store/", IO_MQTT_NODE_ZUMO),
    IO_MQTT_PATH("/sync/", IO_MQTT_NODE_ZUMO)
);

/* Initialize the store connected to MQTT. This class is only used for its
 * constructor, as the store expects the connection to initialized before it is
 * created */
static class mqtt_initer__
{
  public:
    mqtt_initer__()
    {
        LOG_INFO(<< "requesting sync from remote");

        /* Send no data to indicate we are requesting sync data */
        mqtt_gateway_.write("", 0);

        /* Wait to recieve data before we can continue. Only try for X amount of
         * microseconds to prevent hang */
        for (uint64_t start = micros(); micros() - start < SYNC_TIMEOUT_US_;) {
            if (mqtt_gateway_.avail()) {
                return;
            }

            mqtt_gateway_.ps_client()->loop();
        }

        LOG_ERR(<< "error syncing data");
    }
} mqtt_initer_instance__;

store<remote_data> remote_store(
    &mqtt_gateway_,
    (remote_data){
        0,
    }
);

static io::eeprom_gateway eeprom_;

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
