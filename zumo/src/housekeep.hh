
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
    vector<int16_t> accel_meas;
};
extern data_ data;

void update();

}; // namespace hk

#endif // HOUSEKEEP_HH__