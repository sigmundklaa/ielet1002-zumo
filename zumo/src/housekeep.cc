
#include "housekeep.hh"
#include "common.hh"
#include "controller.hh"
#include <logging/log.hh>

#define LOG_MODULE housekeeping
LOG_REGISTER(common::log_gateway);

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

    LOG_DEBUG(
        << "<acceleration> x: " << String(accel[0])
        << ", y: " << String(accel[1]) << ", z: " << String(accel[2])
    );

    data.velocity_calc = calc_vel(data.velocity_calc, accel[0], delta_us);
    data.distance = data.distance + data.velocity_calc * delta_us;

    if (data.velocity_calc > data.velocity_max) {
        data.velocity_max = data.velocity_calc;
    }

    if (data.velocity_calc > data.velocity_max * 0.7) {
        data.velocity_us_above_max += delta_us;
    }

    int16_t* encoder_data = hal::controller.encoder_data();

    ::memcpy(&data.accel_meas, accel, sizeof(*accel) * 3);
}

}; // namespace hk