
#include "common.hh"
#include "comms.hh"
#include "controller.hh"
#include "housekeep.hh"
#include "report.hh"
#include <Arduino.h>
#include <FastGPIO.h>
#include <Wire.h>
#include <io/redirect.hh>
#include <io/serial.hh>
#include <logging/log.hh>
#include <utils/init.hh>

#define LOG_MODULE main
LOG_REGISTER(common::log_gateway);


void
setup()
{
    LOG_INFO(<< "setting up");
}

void
loop()
{
    comms::on_tick();
    common::on_tick();

    hal::on_tick();
    hk::on_tick();
    report::on_tick();

    delay(100);
}
