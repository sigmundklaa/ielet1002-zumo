
#include <Arduino.h>
#include <FastGPIO.h>
#include <Wire.h>
// #include <io/espnow.hh>
#include "common.hh"
#include "comms.hh"
#include <io/redirect.hh>
#include <io/serial.hh>
#include <logging/log.hh>
#include <utils/init.hh>

#define LOG_MODULE main
LOG_REGISTER(common::log_gateway);

static uint8_t redirect_buf_[256];
static io::redirect::redirect_gateway redirect(
    common::serial_gateway_, io::redirect::NODE_MQTT_STORE_1, redirect_buf_
);

void
yield_tick()
{
}

#if 1
void
setup()
{
    LOG_INFO(<< "setting up");
}

struct __attribute__((packed)) t {
    uint8_t x;
    uint16_t y;
    int16_t z;
    int32_t u;
    float w;
} tt;

void
loop()
{
    comms::on_tick();

    t x;
    if (comms::store_gw.read(&x, sizeof(x)) > 0) {
        LOG_INFO(<< "recieved x: " << String(x.x) << ", w: " << String(x.w));
    }
}

#endif