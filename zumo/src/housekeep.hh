
#ifndef HOUSEKEEP_HH__
#define HOUSEKEEP_HH__

#include <stdint.h>

namespace hk
{
struct data_ {
    template <typename T> struct vector {
        T x, y, z;
    };

    /**
     * @brief Calculated velocity, in m/s
     *
     */
    float velocity_calc;

    /**
     * @brief Measured velocity from the encoders
     *
     */
    int16_t velocity_meas;
    int32_t velocity_sum;
    /* How many measurements have been added to the sum. To get the average:
     * velocity_sum / velocity_sum_n */
    int16_t velocity_sum_n;
    int16_t velocity_max;
    uint64_t velocity_s_above_max;

    vector<int16_t> accel_meas;
};
extern data_ data;

void update();

}; // namespace hk

#endif // HOUSEKEEP_HH__