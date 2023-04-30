
#include "control.hh"
#include "autonomy.hh"
#include "common.hh"
#include "comms.hh"
#include "controller.hh"
#include <logging/log.hh>

#define LOG_MODULE control
LOG_REGISTER(common::log_gateway);

namespace control
{
remote_ remote;

remote_::remote_() : auto_enabled_(1) {}

void
remote_::on_tick()
{
    if (comms::control_gw.read(&buf_, sizeof(buf_)) != sizeof(buf_)) {
        return;
    }

    switch (buf_.cmd) {
    case remote_::packet_::CMD_AUTONOMY: {
        uint8_t mode = static_cast<uint8_t>(buf_.arg1);
        LOG_INFO(<< "setting autonomy " << String(mode));

        if (mode != auto_enabled_) {
            autonomy::toggle(mode);
            auto_enabled_ = mode;
        }
        if (!auto_enabled_) {
            /* Ensure the controller is running as autonomy may have stopped it
             */
            hal::controller.start();
        } else {
            hal::controller.stop();
        }

        break;
    }
    case remote_::packet_::CMD_SET_SPEED: {
        LOG_INFO(<< "setting speed");
        if (auto_enabled_) {
            return;
        }

        hal::controller.set_speeds(buf_.arg1, buf_.arg2);

        break;
    }
    }
}

}; // namespace control
