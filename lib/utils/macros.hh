
#ifndef UTILS_MACROS_HH__
#define UTILS_MACROS_HH__

#define UTILS_TRIPLES__(x, y, z) x##y##z
#define UTILS_TRIPLES(x, y, z) UTILS_TRIPLES__(x, y, z)

#define UTILS_CONCAT__(x, y) x##y
#define UTILS_CONCAT(x, y) UTILS_CONCAT__(x, y)

#define UTILS_STR__(x) #x
#define UTILS_STR(x) UTILS_STR__(x)

#define UTILS_ARR_LEN(x) (sizeof(x) / sizeof(x[0]))

#define UTILS_STR_CONCAT(x, y) x y

#endif // UTILS_MACROS_HH__
