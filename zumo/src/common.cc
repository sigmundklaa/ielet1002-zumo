
#include "common.hh"
#include <Arduino.h>
#include <io/eeprom.hh>
#include <logging/log.hh>
#include <utils/init.hh>
#include <utils/new.hh>

#define LOG_MODULE common
LOG_REGISTER(common::log_gateway);

#define SYNC_TIMEOUT_US_ (5e6)

namespace common
{

#define SERIAL_RX_ (0)
#define SERIAL_TX_ (1)

#define STORE_ENDPOINT_ "/store/1"
static io::redirect::redirect_gateway mqtt_gateway_(
    common::serial_gateway_, io::redirect::PACKET_MQTT,
    reinterpret_cast<const uint8_t*>(STORE_ENDPOINT_), sizeof(STORE_ENDPOINT_)
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
#if 0
        for (uint64_t start = micros(); micros() - start < SYNC_TIMEOUT_US_;) {
            if (mqtt_gateway_.avail()) {
                return;
            }

            mqtt_gateway_.ps_client()->loop();
        }
#endif

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
        .batt_status = UINT8_MAX,
        .batt_health = UINT8_MAX,
        .batt_n_charges = 0,
        .batt_n_drained = 0,
    }
);

}; // namespace common
