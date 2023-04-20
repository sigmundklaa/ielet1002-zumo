
#ifndef UTILS_MACROS_HH__
#define UTILS_MACROS_HH__

#define UTILS_CONCAT__(x, y) x##y
#define UTILS_CONCAT(x, y) UTILS_CONCAT__(x, y)

#define UTILS_STR__(x) #x
#define UTILS_STR(x) UTILS_STR__(x)

#define UTILS_STR_CONCAT(x, y) x y

#endif // UTILS_MACROS_HH__
