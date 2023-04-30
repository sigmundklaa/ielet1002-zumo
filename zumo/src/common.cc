
#include "common.hh"
#include "comms.hh"
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

template <typename T> T store<T>::buf;

store<remote_data> remote_store(
    &comms::store_gw,
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

void
on_tick()
{
    static uint8_t local_inited = 0;

    if (!local_inited) {
        local_store.sync();
        local_inited = 1;
    }

    remote_store.sync();
}

}; // namespace common