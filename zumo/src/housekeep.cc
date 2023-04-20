
#include "housekeep.hh"
#include "common.hh"
#include "controller.hh"
#include <logging/log.hh>

#define LOG_MODULE housekeeping
LOG_REGISTER(common::log_sink);

/* https://github.com/pololu/zumo-32u4-arduino-library/blob/master/examples/InertialSensors/InertialSensors.ino
 */
#define ACCEL_CONV_FACTOR_ (0.061e-3f)
#define G_ (9.81f)

#define DISPLAY_INTERVAL_US_ (60e6)

namespace hk
{
data_ data;

/**
 * @brief Calculate velocity
 *
 * @param base Starting velocity
 * @param accel Acceleration
 * @param d_us Delta T in microseconds
 * @return float
 */
static inline float
calc_vel(float base, int16_t accel, int16_t d_us)
{
    return base + ((float)accel * ACCEL_CONV_FACTOR_ * G_) * (d_us / 1e6f);
}

void
update(uint64_t delta_us)
{
    static uint64_t last_display = 0;
    uint64_t tmp = micros();

    if (tmp - last_display >= DISPLAY_INTERVAL_US_) {
    }

    /* TODO: figure out what axis is correct for acceleration */
    int16_t* accel = hal::controller.accel_data();

    data.velocity_calc = calc_vel(data.velocity_calc, accel[0], delta_us);

    ::memcpy(&data.accel_meas, accel, sizeof(*accel) * 3);
}

}; // namespace hk