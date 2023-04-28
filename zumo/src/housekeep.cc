
#include "housekeep.hh"
#include "common.hh"
#include "controller.hh"
#include <logging/log.hh>

#define LOG_MODULE housekeeping
LOG_REGISTER(common::log_gateway);

#define MAX_SPEED_ (80)

namespace hk
{
data_ data;

static inline int16_t
calc_vel_(int16_t cur, int16_t last, uint64_t delta_us)
{
    int32_t cur_c = cur;

    /* If the current has overflowed */
    if (cur_c < 0 && last > 0) {
        cur_c = INT16_MAX + (cur_c - INT16_MIN);
    } else if (cur_c > 0 && last < 0) {
        cur_c = INT16_MIN + (INT16_MAX - cur_c);
    }

    return (cur_c - last); // / delta_us;
}

static void
update_side_(data_::vel& side, int16_t cur, uint64_t delta_us)
{
    side.velocity = calc_vel_(cur, side.last_count, delta_us);
    side.last_count = cur;

    side.distance += abs(side.velocity); // * delta_us;

    if (side.velocity != 0) {
        /* Only track average when not stopped */
        side.vel_sum += side.velocity;
        side.vel_n++;

        if (side.vel_n >= INT32_MAX) {
            side.vel_sum = side.velocity;
            side.vel_n = 1;
        }

        if (side.velocity >= MAX_SPEED_) {
            side.us_above_70p += delta_us;
        }
    }
}

static void
update_(uint64_t delta_us)
{

    int16_t* encoder_data = hal::controller.encoder_data();

    update_side_(data.vel_l, encoder_data[0], delta_us);
    update_side_(data.vel_r, encoder_data[1], delta_us);
}

void
on_tick()
{
    static uint64_t last = 0;
    uint64_t tmp = micros();

    if (tmp - last >= 100e3) {
        update_(tmp - last);

        last = tmp;
    }
}

}; // namespace hk
