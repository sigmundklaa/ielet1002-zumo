
#include "common.hh"
#include <Arduino.h>
#include <Wire.h>
#include <io/mqtt.hh>
#include <io/serial.hh>
#include <logging/log.hh>
#include <stdio.h>

#include "housekeep.hh"
#include <io/serial.hh>

#define LOG_MODULE main
LOG_REGISTER(common::log_sink);

struct __attribute__((packed)) mystruct {
    int x;
    int y;
};

void
setup()
{
    mystruct ms = {
        .x = 3,
        .y = 2,
    };

    common::log_sink.write(&ms, sizeof(ms));
}

void
loop()
{
    hk::update();
}
