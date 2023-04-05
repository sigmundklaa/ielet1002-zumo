
#ifndef ZNEW_HH__
#define ZNEW_HH__

#include <stddef.h>
#include <utils/compile.hh>

#ifndef __unix__

/* Arduino doesn't include an overload for placement new */
inline void*
operator new(unsigned int sz, void* ptr) noexcept__
{
    return ptr;
}

#include_next <new.h>
#else
#include <new>
#endif

#endif // ZNEW_HH__