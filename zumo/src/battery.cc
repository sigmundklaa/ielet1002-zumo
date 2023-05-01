
#include "battery.hh"
#include "common.hh"
#include "comms.hh"
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

template <typename T, typename U, typename W>
static constexpr T
clamp_(T x, U min, W max)
{
    return (x < static_cast<T>(min) ? static_cast<T>(min) : x) >
                   static_cast<T>(max)
               ? static_cast<T>(max)
               : x;
}

struct __attribute__((packed)) charge_request_ {
    uint32_t balance;
};

struct __attribute__((packed)) charge_response_ {
    uint8_t charge;
    uint32_t price;
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
                .balance = common::remote_store.data.bank_currency,
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
    uint8_t factor = 10;
    uint8_t mult =
        ((common::local_store.data.batt_n_drained +
          common::local_store.data.batt_n_charges) /
         5);

    uint8_t cur_status_factor = 0; /*
    (hk::data.velocity_calc +
     (hk::data.velocity_max * 0.7 * (hk::data.velocity_us_above_max / 1000))) /
        2;
    */

    uint8_t total = factor * mult + cur_status_factor / 10;

    /* Battery production error */
    if (random(0, 100) == 0) {
        total *= 2;
    }

    return total;
}

uint8_t
battery__::calc_drain_batt_()
{
    uint8_t factor = 10;
    uint8_t mult = (((hk::data.vel_l.distance + hk::data.vel_r.distance) / 2) *
                    calc_vel_avg_()) /
                   50;

    return mult * factor;
}

void
battery__::service()
{
    LOG_DEBUG(<< "servicing battery");

    common::local_store.data.batt_health =
        u8_clamp_(common::local_store.data.batt_health * 2);

    if (common::local_store.data.batt_health > HEALTH_LEVEL_LOW_) {
        health_state_ = HEALTH_NORM_;
    } else if (common::local_store.data.batt_health > HEALTH_LEVEL_CRIT_) {
        health_state_ = HEALTH_CRIT_;
    }
}

void
battery__::replace()
{
    LOG_DEBUG(<< "replacing battery");

    health_state_ = HEALTH_LOW_;
    common::local_store.data.batt_health = UINT8_MAX;
}

void
battery__::charge()
{
    common::local_store.data.batt_n_charges++;

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
    common::local_store.data.batt_health =
        u8_clamp_(common::local_store.data.batt_health - calc_drain_health_());

    uint8_t health = common::local_store.data.batt_health;

    switch (health_state_) {
    case HEALTH_NORM_:
        if (health < HEALTH_LEVEL_LOW_) {
            health_state_ = HEALTH_LOW_;
        }
    case HEALTH_LOW_:
        if (health < HEALTH_LEVEL_CRIT_) {
            health_state_ = HEALTH_CRIT_;
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

        common::local_store.data.batt_status = clamp_<uint8_t>(
            common::local_store.data.batt_status + charge_buf_.response.charge,
            0, UINT8_MAX
        );
        common::remote_store.data.bank_currency = clamp_<uint32_t>(
            common::remote_store.data.bank_currency -
                charge_buf_.response.price,
            0, UINT32_MAX
        );

        transition_(STATE_INACTIVE_);
        break;
    }
    case STATE_DRAINING_: {
        uint64_t tmp = micros();

        if (tmp - time_last_us_ < 100e3) {
            break;
        }

        common::local_store.data.batt_status = u8_clamp_(
            common::local_store.data.batt_status - calc_drain_batt_()
        );

        common::local_store.data.batt_health = u8_clamp_(
            common::local_store.data.batt_health - calc_drain_health_()
        );

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