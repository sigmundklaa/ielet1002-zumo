
#include "common.hh"
#include <Arduino.h>
#include <io/mqtt.hh>
#include <io/serial.hh>
#include <logging/log.hh>
#include <stdio.h>

#include "housekeep.hh"

#define LOG_MODULE main
LOG_REGISTER(&common::log_sink);

void
setup()
{
}

void
loop()
{
    hk::update();
}
