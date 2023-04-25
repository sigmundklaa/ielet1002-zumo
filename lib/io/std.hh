
#ifndef IO_STD_HH__
#define IO_STD_HH__

#if !defined(MCU__) || !MCU__

#include <assert.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>

#include "io.hh"
#include <utils/compile.hh>

namespace io
{

/**
 * @brief Redirects to stdout. This is only for testing, and should not be
 * compiled in for Arduino code.
 * When compiled for the host machine this can be used as a replacement for
 * the Serial gateway.
 */
class std_gateway : public gateway
{
  protected:
    size_t
    write_(const uint8_t* data, size_t size) override__
    {
        const char* str = reinterpret_cast<const char*>(data);
        return ::printf("%.*s", static_cast<int>(size), str);
    }

    size_t
    read_(uint8_t* buf, size_t buf_size) override__
    {
        char* strbuf = reinterpret_cast<char*>(buf);

        std::cin.getline(reinterpret_cast<char*>(buf), buf_size);
        return ::strnlen(strbuf, buf_size);
    }
};

}; // namespace io

#endif

#endif // IO_STD_HH__
