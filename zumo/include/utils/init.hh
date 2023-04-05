
#ifndef UTILS_INIT_HH__
#define UTILS_INIT_HH__

#include "mem.hh"
#include <stdint.h>

namespace utils
{
template <typename T> class init_guard
{
  protected:
    static int nifty_;
    static uint8_t mem_[aligned__(sizeof(T), alignof__(T))];

  public:
    typedef void (*constructor)(T&);

    init_guard(constructor c);
};
}; // namespace utils

#endif // UTILS_INIT_HH__