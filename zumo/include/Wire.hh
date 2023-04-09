/* This file ensures that Wire.begin() is called when including the Wire.h file.
 * That way we dont need to worry about either Wire.h not being initialized, or
 * being initialized too many times.
 *
 * Ideally this would be named Wire.h and include_next would be used, but for
 * some reason the Zumo library will not compile when using that. */

#ifndef ZWIRE_HH__
#define ZWIRE_HH__

#include <Wire.h>
#include <utils/init.hh>

namespace wire_init__
{
class dummy__
{
};

inline void
init__(dummy__& mem_)
{
    Wire.begin();
}

static dummy__& d__ = init_guarded(dummy__, init__);
}; // namespace wire_init__

#endif // ZWIRE_HH__