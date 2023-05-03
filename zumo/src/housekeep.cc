
#include "housekeep.hh"
#include "common.hh"
#include "controller.hh"
#include <logging/log.hh>

#define LOG_MODULE housekeeping
LOG_REGISTER(common::log_gateway);

#define MAX_SPEED_ (100)

namespace hk
{
data_ data;

/**
 * @brief Calculate the velocity based on the delta between the last measurement
 * from the encoders, given through @p last, and current measurement given
 * through @p cur
 *
 * @param cur Current measurement
 * @param last Previous measurement
 * @param delta_us Delta in microseconds between measurements
 * @return int16_t Velocity
 */
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

/**
 * @brief Helper function to update housekeeping data for only one side of the
 * Zumo.
 *
 * @param side
 * @param cur Current encoder reading for the appropriate side
 * @param delta_us Delta in microseconds between measurements
 */
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

        if (side.velocity > side.vel_max) {
            side.vel_max = side.velocity;
        }
    }
}

/**
 * @brief Updates both sides based on the current reading of the encoders
 *
 * @param delta_us Delta in microseconds since last update
 */
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
