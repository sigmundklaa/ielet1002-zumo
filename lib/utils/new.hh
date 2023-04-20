
#ifndef UTILS_NEW_HH__
#define UTILS_NEW_HH__

#include <stddef.h>
#include <utils/compile.hh>

#if defined(MCU__) && MCU__

/* Arduino (atleast ESP32 in particular) doesn't include an overload for
 * placement new */
inline void*
operator new(unsigned int sz, void* ptr) noexcept__
{
    return ptr;
}

#endif

#include <new>

#endif // UTILS_NEW_HH__