
#include "common.hh"
#include <Arduino.h>
#include <Wire.h>
// #include <io/espnow.hh>
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
    LOG_ERR(<< "test");
    LOG_INFO(<< "setting up");
    LOG_ERR(<< "test");
    LOG_ERR(<< "test");
    LOG_SAVE();
}

void
loop()
{
}
