
#ifndef ZNEW_HH__
#define ZNEW_HH__

#include <stddef.h>

#ifndef __unix__

/* Arduino doesn't include an overload for placement new */
inline void*
operator new(unsigned long sz, void* ptr) throw()
{
    return ptr;
}

#include_next <new.h>
#else
#include <new>
#endif

#endif // ZNEW_HH__