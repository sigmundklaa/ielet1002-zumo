
#ifndef UTILS_COMPILE_HH__
#define UTILS_COMPILE_HH__

#if __cplusplus >= 201103L
#define override__ override
#define noexcept__ noexcept
#else
#define override__
#define noexcept__ throw()
#endif
#endif // UTILS_COMPILE_HH__