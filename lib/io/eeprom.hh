
#ifndef EEPROM_HH__
#define EEPROM_HH__

#include "io.hh"
#include <utils/compile.hh>
#include <utils/crc32.hh>

/*
While we could use the Arduino built-in library EEPROM.h here, it is
unneccessary and less fitting for our use case. This code will only run on an
ESP32 anyways so there is no need to worry we are running on anything other than
an AVR core. Using the AVR eeprom.h library directly allows us to write/read the
entire buffer at once, in contrast to the Arduino library which reads byte by
byte. This also saves us from the overhead of the overloaded operators that make
the EEPROM.h library.
*/
#include <avr/eeprom.h>

#define CHECKSUM_SZ_ (sizeof(uint32_t))

namespace io
{

/**
 * @brief Implements the gateway interface for the eeprom. Should only have one
 * user, as write and read always uses index 0 + sizeof(checksum). The checksum
 * is a 32bit CRC32 checksum.
 *
 */
class eeprom_gateway : public gateway
{
  protected:
    uint32_t checksum_buf_;

    size_t
    write_(const uint8_t* data, size_t size) override__
    {
        checksum_buf_ = utils::crc32(data, size);

        ::eeprom_write_block(data, (void*)0, sizeof(checksum_buf_));
        ::eeprom_write_block(data, (void*)CHECKSUM_SZ_, size);

        return size;
    }

    size_t
    read_(uint8_t* buf, size_t buf_size) override__
    {
        ::eeprom_read_block(&checksum_buf_, (void*)0, sizeof(checksum_buf_));
        ::eeprom_read_block(buf, (void*)CHECKSUM_SZ_, buf_size);

        if (utils::crc32(buf, buf_size) != checksum_buf_) {
            return 0;
        }

        return buf_size;
    }
};
}; // namespace io

#endif // EEPROM_HH__