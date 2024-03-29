
#include "controller.hh"
#include "battery.hh"
#include "common.hh"
#include <Arduino.h>
#include <Wire.h>
#include <Zumo32U4.h>
#include <logging/log.hh>
#include <utils/trace.hh>

#define LOG_MODULE controller
LOG_REGISTER(common::log_gateway);

#define DIR_CHANGE_DELAY_US_ (100e3)
#define READ_INTERVAL_US_ (50e3)

namespace hal
{

static struct {
    Zumo32U4Encoders encoders;
    Zumo32U4LineSensors lines;
} components_;

controller_ controller;

controller_::side_::side_(side_num sn)
    : cur_state_(STATE_STOPPED_), side_num_(sn)
{
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
            cur_state_ = STATE_IDLE_;
            start();
            return;
        }
    }
    case STATE_STOPPED_:
        set_motor_speed_(0);
        break;
    case STATE_RUNNING_:
        set_motor_speed_(speed_);
        break;
    case STATE_IDLE_:
        break;
    }
}

void
controller_::side_::set_motor_speed_(uint8_t s)
{
    /* Convert to the 0-400 range that the Zumo expects. We use the range 0-255
    because there is not really any good reason to use 0-400 outside of the
    value set in the register, which is handled by the Zumo32U4 functions. */
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
    swbat::battery.toggle_drain(1);
}

void
controller_::side_::stop_()
{
    stop_time_us_ = micros();
    swbat::battery.toggle_drain(0);
}

void
controller_::side_::transition_(side_::state_ st)
{
    /*LOG_DEBUG(
        << "<transition> side: " << String(static_cast<int>(side_num_))
        << ", state: " << String(static_cast<int>(cur_state_))
        << ", new state: " << String(static_cast<int>(st))
    );*/

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
        /* To prevent damage to the motors there needs to be a delay between
         * switching directions. */
        if (stop_time_us_ >= start_time_us_ ||
            (speed_ == 0 && micros() - stop_time_us_ >= DIR_CHANGE_DELAY_US_)) {
            break;
        }
    case STATE_STOPPED_:
        stop_();
        break;
    case STATE_IDLE_:
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
controller_::side_::set_speed_noabs(int16_t speed)
{
    side_::direction dir = speed < 0 ? DIR_BWARD : DIR_FWARD;

    this->set_dir(dir);
    this->set_speed(static_cast<uint8_t>(abs(speed)));
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

void
controller_::read_sensors_()
{
    TRACE_ENTER(__func__);

    readings_.encoder[0] = components_.encoders.getCountsLeft();
    readings_.encoder[1] = components_.encoders.getCountsRight();

    TRACE_EXIT(__func__);
}

controller_::controller_()
    : left(controller_::side_::LEFT), right(controller_::side_::RIGHT)
{
}

void
controller_::set_speeds(int16_t l, int16_t r)
{
    left.set_speed_noabs(l);
    right.set_speed_noabs(r);
}

void
controller_::stop()
{
    left.stop();
    right.stop();
}

void
controller_::start()
{
    left.start();
    right.start();
}

void
controller_::init_()
{
    Wire.begin();

    components_.lines.initFiveSensors();
}

void
controller_::run()
{
    if (!inited_) {
        init_();
        inited_ = 1;
    }

    uint64_t tmp = micros();
    uint64_t delta = tmp - last_read_us_;
    if (delta >= READ_INTERVAL_US_) {
        read_sensors_();

        button_b.handle(delta);
        button_c.handle(delta);

        last_read_us_ = tmp;
    }

    /* Line readings need to be more frequent than the read interval for the
     * rest of the hardware. */
    readings_.position = components_.lines.readLine(readings_.lines);

    left.run();
    right.run();
}

void
controller_::calibrate()
{
    components_.lines.calibrate();
}

}; // namespace hal