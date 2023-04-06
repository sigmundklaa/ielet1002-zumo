
#ifndef UTILS_INIT_HH__
#define UTILS_INIT_HH__

#include "mem.hh"
#include <stdint.h>

namespace utils
{

/**
 * @brief Helper class to ensure correct initialization order. This is useful
 * when one the constructor of one object expects that another object has been
 * properly constructed before using it. An example of this is the MQTT client:
 * Some object X may want to subscribe to a particular topic in its constructor
 * so it calls .subscribe on the mqtt client. However there is no guarantee that
 * the MQTT client has been initialized yet, so attempting to use it could lead
 * to a segmentation fault.
 *
 * @tparam T Type of
 */
template <typename T> class init_guard
{
  protected:
    static int nifty_;

  public:
    static uint8_t mem[aligned__(sizeof(T), alignof__(T))];
    typedef void (*constructor)(T&);

    init_guard(constructor c);
};
}; // namespace utils

/* Helper macro to hide the ugliness of creating an init guard. */
#define init_guarded(type, cons)                                               \
    (utils::init_guard<type>(cons),                                            \
     reinterpret_cast<type&>(utils::init_guard<type>::mem))

#include "init.icc"

#endif // UTILS_INIT_HH__