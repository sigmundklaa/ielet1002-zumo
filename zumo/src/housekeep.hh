
#ifndef HOUSEKEEP_HH__
#define HOUSEKEEP_HH__

#include <stdint.h>

namespace hk
{
struct data_ {
    template <typename T> struct vector {
        T x, y, z;
    };

    struct vel {
        int16_t velocity;
        int16_t last_count;
        int16_t vel_max;
        int32_t vel_sum;
        int32_t vel_n;

        int32_t distance;
        uint64_t us_above_70p;
    } vel_l, vel_r;
};
extern data_ data;

void on_tick();

}; // namespace hk

#endif // HOUSEKEEP_HH__