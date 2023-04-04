
#ifndef AVR_EEPROM_H__
#define AVR_EEPROM_H__

#include <stddef.h>
#include <stdint.h>

void
eeprom_write_block(const void* __src, void* __dst, size_t __n)
{
}

void
eeprom_read_block(void* __dst, const void* __src, size_t __n)
{
}

#endif // AVR_EEPROM_H__