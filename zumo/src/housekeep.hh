
#ifndef HOUSEKEEP_HH__
#define HOUSEKEEP_HH__

#include <stdint.h>

namespace hk
{
struct data_ {
    template <typename T> struct vector {
        T x, y, z;
    };

    float velocity;

    float velocity_sum;

    /* How many measurements have been added to the sum. To get the average:
     * velocity_sum / velocity_sum_n */
    float velocity_sum_n;

    float velocity_max;

    vector<int16_t> accel_meas;
};
extern data_ data;

void update();

}; // namespace hk

#endif // HOUSEKEEP_HH__