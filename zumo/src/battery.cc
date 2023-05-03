
#include "battery.hh"
#include "common.hh"
#include "comms.hh"
#include "controller.hh"
#include "housekeep.hh"
#include <Arduino.h>
#include <io/serial.hh>
#include <logging/log.hh>

#define LOG_MODULE sw_battery
LOG_REGISTER(common::log_gateway);

#define HEALTH_LEVEL_LOW_ (UINT8_MAX * 0.1)
#define HEALTH_LEVEL_CRIT_ (UINT8_MAX * 0.05)

#define CHARGE_THRESHHOLD_ (UINT8_MAX * 0.2)

namespace swbat
{

template <typename T, typename U, typename W, typename X>
static constexpr T
clamp_(U x, W min, X max)
{
    return static_cast<T>(x > max ? max : (x < min ? min : x));
}

typedef void (*button_hold_fn_)();

static void
start_service_()
{
    battery.service();
}

static void
start_change_()
{
    battery.replace();
}

/**
 * @brief
 *
 * @tparam uint8_t
 */
template <button_hold_fn_ callback>
static void
hold_buttons_()
{
    static uint64_t last_press = 0;
    uint64_t tmp = micros();

    /*  */
    if (tmp - last_press < 100e3) {
#if 0
        common::local_store.data.batt_health = 255;
        common::local_store.data.batt_status = 255;
#else
        callback();
#endif
    } else {
        last_press = tmp;
    }
}

struct __attribute__((packed)) charge_request_ {
    uint8_t status;
    uint8_t health;
};

struct __attribute__((packed)) charge_response_ {
    uint8_t status;
    uint8_t health;
    uint8_t cycles;
};

static union {
    charge_request_ request;
    charge_response_ response;
} charge_buf_;

battery__ battery;

static int32_t
calc_vel_avg_()
{
    return ((hk::data.vel_l.vel_sum / hk::data.vel_l.vel_n) +
            (hk::data.vel_r.vel_sum / hk::data.vel_r.vel_n)) /
           2;
}

static uint8_t
u8_clamp_(int x)
{
    return clamp_<uint8_t>(x, 0, UINT8_MAX);
}

void
on_init()
{
    hal::controller.button_b.set_3s_callback(hold_buttons_<start_change_>);
    hal::controller.button_c.set_3s_callback(hold_buttons_<start_change_>);

    hal::controller.button_b.set_1s_callback(hold_buttons_<start_service_>);
    hal::controller.button_c.set_1s_callback(hold_buttons_<start_service_>);
}

void
battery__::transition_(enum state new_state)
{
    switch (state_) {
    case STATE_CHARGING_:
        if (new_state != STATE_INACTIVE_) {
            LOG_ERR(<< "illegal transition");
            return;
        }
    case STATE_INACTIVE_:
        if (new_state == STATE_DRAINING_) {
            time_last_us_ = micros();
        }
    case STATE_DRAINING_:
        if (new_state == STATE_CHARGING_) {
            charge_buf_.request = (charge_request_){
                .status = common::local_store.data.batt_status,
                .health = common::local_store.data.batt_health,
            };

            comms::charge_gw.write(
                &charge_buf_.request, sizeof(charge_buf_.request)
            );
            break;
        }
        break;
    }

    state_ = new_state;
}

uint8_t
battery__::calc_drain_health_()
{
    uint8_t battdmg =
        ((common::local_store.data.batt_n_drained +
          common::local_store.data.batt_n_charges) /
         5);

    uint8_t total = battdmg +
                    (hk::data.vel_l.velocity / 200 +
                     (400 * 0.7 * (hk::data.vel_l.us_above_70p / 1e6)) / 4000 +
                     hk::data.vel_r.velocity / 200 +
                     (400 * 0.7 * (hk::data.vel_r.us_above_70p / 1e6)) / 4000) /
                        2;

    /* Battery production error */
    if (random(0, 100) == 0) {
        total *= 2;
    }

    return total;
}

uint8_t
battery__::calc_drain_batt_()
{
    return (((hk::data.vel_l.distance + hk::data.vel_r.distance) / 20000) *
            calc_vel_avg_()) /
           100;
}

void
battery__::service()
{
    LOG_DEBUG(<< "servicing battery");

    transition_(STATE_CHARGING_);
}

void
battery__::replace()
{
    LOG_DEBUG(<< "replacing battery");

    transition_(STATE_CHARGING_);
}

void
battery__::charge()
{
    transition_(STATE_CHARGING_);
}

uint8_t
battery__::need_charge()
{
    return common::local_store.data.batt_status > CHARGE_THRESHHOLD_;
}

void
battery__::on_tick()
{
    uint8_t health = common::local_store.data.batt_health;

    switch (health_state_) {
    case HEALTH_NORM_:
        if (health < HEALTH_LEVEL_LOW_) {
            health_state_ = HEALTH_LOW_;
        }
    case HEALTH_LOW_:
        if (health < HEALTH_LEVEL_CRIT_) {
            health_state_ = HEALTH_CRIT_;
            common::local_store.data.batt_n_drained++;
        }
    case HEALTH_CRIT_:
        break;
    }

    switch (state_) {
    case STATE_CHARGING_: {
        size_t bread = comms::charge_gw.read(
            &charge_buf_.response, sizeof(charge_buf_.response)
        );

        if (bread == 0) {
            break;
        }

        common::local_store.data.batt_status = charge_buf_.response.status;
        common::local_store.data.batt_health = charge_buf_.response.health;

        if (common::local_store.data.batt_health < HEALTH_LEVEL_CRIT_) {
            health_state_ = HEALTH_CRIT_;
        } else if (common::local_store.data.batt_health < HEALTH_LEVEL_LOW_) {
            health_state_ = HEALTH_LOW_;
        } else {
            health_state_ = HEALTH_NORM_;
        }

        common::local_store.data.batt_n_charges += charge_buf_.response.cycles;

        transition_(STATE_INACTIVE_);
        break;
    }
    case STATE_DRAINING_: {
        uint64_t tmp = micros();

        if (tmp - time_last_us_ < 100e3) {
            break;
        }

        if (tmp - time_last_drain_us_ >= 5e6) {
            common::local_store.data.batt_status = clamp_<uint8_t>(
                static_cast<int16_t>(common::local_store.data.batt_status) -
                    calc_drain_batt_(),
                0, UINT8_MAX
            );

            common::local_store.data.batt_health = clamp_<uint8_t>(
                static_cast<int16_t>(common::local_store.data.batt_health) -
                    calc_drain_health_(),
                0, UINT8_MAX
            );

            time_last_drain_us_ = tmp;
        }

        time_active_us_ += tmp - time_last_us_;
        time_last_us_ = tmp;

        break;
    }
    case STATE_INACTIVE_:
        break;
    }
}

void
battery__::toggle_drain(uint8_t draining)
{
    transition_(draining ? STATE_DRAINING_ : STATE_INACTIVE_);
}
}; // namespace swbat