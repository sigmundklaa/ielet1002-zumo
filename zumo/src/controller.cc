
#include "controller.hh"
#include "common.hh"
#include <Arduino.h>
#include <Zumo32U4Motors.h>
#include <logging/log.hh>

#define LOG_MODULE controller
LOG_REGISTER(&common::log_sink);

#define DIR_CHANGE_DELAY_US_ (100e3)

namespace hal
{

controller_ controller;

controller_::side_::side_() : cur_state_(STATE_STOPPED_) {}

void
controller_::side_::assign_side(controller_::side_::side_num s)
{
    side_num_ = s;
}

void
controller_::side_::run()
{
    switch (cur_state_) {
    case STATE_CHANGE_DIR_: {
        uint64_t tmp = micros();

        /* A delay is required before we can change direction, otherwise the
         * motors can be destroyed */
        if (tmp - stop_time_us_ >= DIR_CHANGE_DELAY_US_) {
            start();
            return;
        }

        break;
    }
    case STATE_RUNNING_:
    case STATE_STOPPED_:
        break;
    }
}

void
controller_::side_::set_motor_speed_(uint8_t s)
{
    /* Convert to the 0-400 range that the Zumo expects. We use the range 0-255
    because there is not really any good reason to use 0-400. */
    int16_t conv = map(s, 0, 255, 0, 400);

    if (direction_ == DIR_BWARD) {
        conv = -conv;
    }

    switch (side_num_) {
    case LEFT:
        Zumo32U4Motors::setLeftSpeed(conv);
        break;
    case RIGHT:
        Zumo32U4Motors::setRightSpeed(conv);
        break;
    }
}

void
controller_::side_::start_()
{
    start_time_us_ = micros();

    set_motor_speed_(speed_);
}

void
controller_::side_::stop_()
{
    stop_time_us_ = micros();

    set_motor_speed_(0);
}

void
controller_::side_::transition_(side_::state_ st)
{
    /* We are changing direction so we need to wait until we are done with that
     * before we can transition to any other state. TODO: either error return or
     * queue transition */
    if (cur_state_ == STATE_CHANGE_DIR_) {
        return;
    }

    switch (st) {
    case STATE_RUNNING_:
        start_();
        break;
    case STATE_CHANGE_DIR_:
    case STATE_STOPPED_:
        stop_();
        break;
    };

    cur_state_ = st;
}

void
controller_::side_::set_dir(side_::direction dir)
{
    if (dir == direction_) {
        return;
    }

    direction_ = dir;
    transition_(STATE_CHANGE_DIR_);
}

void
controller_::side_::set_speed(uint8_t speed)
{
    speed_ = speed;
}

void
controller_::side_::stop()
{
    transition_(STATE_STOPPED_);
}

void
controller_::side_::start()
{
    transition_(STATE_RUNNING_);
}

uint8_t
controller_::side_::running()
{
    return cur_state_ == STATE_RUNNING_;
}

controller_::controller_()
{
    sides_[0].assign_side(controller_::side_::LEFT);
    sides_[1].assign_side(controller_::side_::RIGHT);
}

void
controller_::run()
{
    for (size_t i = 0; i < 2; i++) {
        sides_[i].run();
    }
}

}; // namespace hal