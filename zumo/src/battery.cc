
#include "battery.hh"
#include "common.hh"
#include <Arduino.h>
#include <io/serial.hh>
#include <logging/log.hh>

#define LOG_MODULE sw_battery
LOG_REGISTER(common::log_sink);

namespace swbat
{
battery__ battery;
};