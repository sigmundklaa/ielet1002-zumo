
#ifndef MEM_ALIGN_HH__
#define MEM_ALIGN_HH__

#include <stddef.h>

#if __cplusplus < 201103L
namespace mem
{

/*
Implementation of a custom alignof operator, as the alignof operator is not
supported in C++03. This implementation is based on the implementation described
here: http://www.wambold.com/Martin/writings/alignof.html
*/

template <typename T> struct alignof_1__ {
    enum { s = sizeof(T), value = s ^ (s & (s - 1)) };
};

template <typename T> struct alignof_2__;

template <size_t diff> struct helper__ {
    template <typename T> struct val {
        enum { value = diff };
    };
};

template <> struct helper__<0> {
    template <typename T> struct val {
        enum { value = alignof_2__<T>::value };
    };
};

template <typename T> struct alignof_2__ {
    struct bigger {
        T obj;
        char c;
    };

    enum {
        diff = sizeof(bigger) - sizeof(T),
        value = helper__<diff>::template val<T>::value
    };
};

template <typename T> struct alignof_3__ {
    enum {
        x = alignof_1__<T>::value,
        y = alignof_2__<T>::value,
        value = (int)x < (int)y ? x : y
    };
};

}; // namespace mem

#define alignof__(x) (mem::alignof_3__<x>::value)
#define alignas__(x) __attribute__((aligned(x)))
#else
#define alignof__(x) alignof(x)
#define alignas__(x) alignas(x)
#endif

#define aligned__(size, alignment) ((size + (alignment - 1)) & ~(alignment - 1))

#endif // MEM_ALIGN_HH__