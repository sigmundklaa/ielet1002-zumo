
#include "common.hh"
#include <Arduino.h>
#include <Wire.h>
// #include <io/espnow.hh>
#include <io/serial.hh>
#include <logging/log.hh>
#include <stdio.h>

#include "housekeep.hh"
#include <io/serial.hh>

#define LOG_MODULE main
LOG_REGISTER(common::log_gateway);

void
yield_tick()
{
}

#if 1
void
setup()
{
}

void
loop()
{
    LOG_ERR(<< "test");
    LOG_INFO(<< "setting up");
    LOG_ERR(<< "test");
    LOG_ERR(<< "test");
    LOG_SAVE();
    delay(1000);
    yield_tick();
}

#endif