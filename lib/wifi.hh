
#ifndef WIFI_HH__
#define WIFI_HH__

#include <WiFi.h>
#include <utils/init.hh>

namespace wifi
{
struct init_dummy__ {
};

static inline void
init__(init_dummy__& mem)
{
    WiFi.mode(WIFI_STA);
}

static init_dummy__& wid__ = init_guarded(init_dummy__, init__);
}; // namespace wifi

#endif // WIFI_HH__