
#include "housekeep.hh"
#include "common.hh"
#include <logging/log.hh>

#define LOG_MODULE housekeeping
LOG_REGISTER(&common::log_sink);

/* https://github.com/pololu/zumo-32u4-arduino-library/blob/master/examples/InertialSensors/InertialSensors.ino
 */
#define ACCEL_CONV_FACTOR_ (0.061e-3f)
#define G_ (9.81f)

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
update()
{
    data = (data_){

    };
}

}; // namespace hk