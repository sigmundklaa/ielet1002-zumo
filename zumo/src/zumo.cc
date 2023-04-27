
#include <Arduino.h>
#include <FastGPIO.h>
#include <Wire.h>
// #include <io/espnow.hh>
#include "common.hh"
#include "comms.hh"
#include "zumo.hh"
#include <io/redirect.hh>
#include <io/serial.hh>
#include <logging/log.hh>
#include <utils/init.hh>

#define LOG_MODULE main
LOG_REGISTER(common::log_gateway);

void
zumo::yield_tick()
{
    comms::on_tick();
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
    zumo::yield_tick();
    common::on_tick();

    LOG_INFO(
        << "x: " << String(common::remote_store.data.x)
        << ", w: " << String(common::remote_store.data.w)
    );
    delay(500);
}

#endif