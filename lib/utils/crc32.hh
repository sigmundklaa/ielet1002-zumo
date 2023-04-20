
#ifndef UTILS_CRC32_HH__
#define UTILS_CRC32_HH__

#include <stddef.h>
#include <stdint.h>

namespace utils
{

/**
 * @brief CRC32 algorithm, without using an expensive lookup table. Slower than
 * with a table but more suited for MCUs with minimal RAM. Taken from:
 * https://stackoverflow.com/questions/21001659/crc32-algorithm-implementation-in-c-without-a-look-up-table-and-with-a-public-li
 *
 * @param buf
 * @param size
 * @return uint32_t
 */
uint32_t
crc32(const uint8_t* buf, size_t size)
{
    uint32_t crc = 0xFFFFFFFF;

    for (uint_fast8_t i = 0; i < size && buf[i]; i++) {
        crc ^= buf[i];
        for (uint_fast8_t j = 7; j >= 0; j--) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }

    return ~crc;
}

}; // namespace utils

#endif // UTILS_CRC32_HH__