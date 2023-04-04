
#include "battery.hh"
#include <Arduino.h>
#include <io/serial.hh>
#include <logging/log.hh>

static io::serial_sink log_sink_;

#define LOG_MODULE sw_battery
LOG_REGISTER(&log_sink_);

namespace swbat
{
battery__ battery;
};